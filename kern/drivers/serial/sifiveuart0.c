#include "sifiveuart0.h"
#include "serial.h"
#include <endian.h>
#include <kern/drivers/device.h>
#include <kern/drivers/intc.h>
#include <kern/drivers/serialport.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lib/sync.h>
#include <kern/mm/kalloc.h>
#include <kern/mm/pmm.h>
#include <kern/mm/vmm.h>

struct sifiveuart0 {
  char *su_name;
  uintptr_t su_addr;
  uintmax_t su_size;
  struct {
    uint8_t *buf_data;
    size_t buf_off_b;
    size_t buf_off_e;
    size_t buf_size;
  } su_rd_buf, su_wr_buf;
};

static inline void sifiveuart0_write(struct sifiveuart0 *uart, unsigned long addr,
                                     uint32_t data) {
  fence(w, o);
  *((volatile uint32_t *)(uart->su_addr + addr)) = data;
}

static inline uint32_t sifiveuart0_read(struct sifiveuart0 *uart, unsigned long addr) {
  uint32_t c = *((volatile uint32_t *)(uart->su_addr + addr));
  fence(i, r);
  return c;
}

static void sifiveuart0_init(struct sifiveuart0 *uart) {
  sifiveuart0_write(uart, COM_TXCTRL,
                    COM_TXCTRL_TXEN | (1 << COM_TXCTRL_TXCNT_SHIFT)); // enable tx
  sifiveuart0_write(uart, COM_RXCTRL, COM_RXCTRL_RXEN);               // enable rx
  sifiveuart0_write(uart, COM_IE, COM_IPE_RXWM); // enable receive data interrupt
}

static int sifiveuart0_getc(void *ctx, uint8_t *c) {
  struct sifiveuart0 *uart = ctx;
  if (uart->su_rd_buf.buf_size > 0) {
    uint8_t data = uart->su_rd_buf.buf_data[uart->su_rd_buf.buf_off_b];
    *c = data == 0xffU ? 0 : data;
    uart->su_rd_buf.buf_off_b = (uart->su_rd_buf.buf_off_b + 1) % SERIAL_BUFFER_SIZE;
    uart->su_rd_buf.buf_size--;
    return 1;
  }
  uint32_t rxdata = sifiveuart0_read(uart, COM_RXDATA);
  if ((rxdata & COM_RXDATA_EMPTY) != 0) {
    return 0;
  }
  uint8_t data = rxdata & 0xffU;
  *c = data == 0xffU ? 0 : data;
  return 1;
}

static int sifiveuart0_putc(void *ctx, uint8_t c) {
  struct sifiveuart0 *uart = ctx;
  if (uart->su_wr_buf.buf_size < SERIAL_BUFFER_SIZE) {
    uart->su_wr_buf.buf_off_e = (uart->su_wr_buf.buf_off_e + 1) % SERIAL_BUFFER_SIZE;
    uart->su_wr_buf.buf_data[uart->su_wr_buf.buf_off_e] = c;
    uart->su_wr_buf.buf_size++;
    sifiveuart0_write(uart, COM_IE, COM_IPE_RXWM | COM_IPE_TXWM);
    return 1;
  }
  return 0;
}

static void sifiveuart0_flush(void *ctx) {
  struct sifiveuart0 *uart = ctx;
  while (uart->su_wr_buf.buf_size > 0) {
    while ((sifiveuart0_read(uart, COM_TXDATA) & COM_TXDATA_FULL) != 0) {
    }
    sifiveuart0_write(uart, COM_TXDATA, uart->su_wr_buf.buf_data[uart->su_wr_buf.buf_off_b]);
    uart->su_wr_buf.buf_off_b = (uart->su_wr_buf.buf_off_b + 1) % SERIAL_BUFFER_SIZE;
    uart->su_wr_buf.buf_size--;
  }
}

static long sifiveuart0_handle_int(void *ctx, unsigned long trap_num) {
  struct sifiveuart0 *uart = ctx;
  uint8_t ip = sifiveuart0_read(uart, COM_IP);
  if (ip & COM_IPE_RXWM) {
    uint32_t data;
    while (((data = sifiveuart0_read(uart, COM_RXDATA)) & COM_RXDATA_EMPTY) != 0) {
      if (uart->su_rd_buf.buf_size == SERIAL_BUFFER_SIZE) {
        break;
      }
      uart->su_rd_buf.buf_off_e = (uart->su_rd_buf.buf_off_e + 1) % SERIAL_BUFFER_SIZE;
      uart->su_rd_buf.buf_data[uart->su_rd_buf.buf_off_e] = data & 0xffU;
      uart->su_rd_buf.buf_size++;
    }
  }
  if (ip & COM_IPE_TXWM) {
    while ((sifiveuart0_read(uart, COM_TXDATA) & COM_TXDATA_FULL) == 0) {
      if (uart->su_wr_buf.buf_size == 0) {
        break;
      }
      sifiveuart0_write(uart, COM_TXDATA, uart->su_wr_buf.buf_data[uart->su_wr_buf.buf_off_b]);
      uart->su_wr_buf.buf_off_b = (uart->su_wr_buf.buf_off_b + 1) % SERIAL_BUFFER_SIZE;
      uart->su_wr_buf.buf_size--;
    }
    if (uart->su_wr_buf.buf_size == 0) {
      sifiveuart0_write(uart, COM_IE, COM_IPE_RXWM);
    } else {
      sifiveuart0_write(uart, COM_IE, COM_IPE_RXWM | COM_IPE_TXWM);
    }
  }
  return KER_SUCCESS;
}

static int sifiveuart0_pred(const struct dev_node *node) {
  struct dev_node_prop *prop = dt_node_prop_extract(node, "compatible");
  return prop != NULL && dt_match_strlist(prop->pr_values, prop->pr_len, "sifive,uart0");
}

static long sifiveuart0_probe(const struct dev_node *node) {
  struct dev_node_prop *prop;
  prop = dt_node_prop_extract(node, "reg");
  if (prop == NULL) {
    return -KER_DTB_ER;
  }
  uint64_t addr = from_be(*((uint64_t *)prop->pr_values));
  uint64_t size = from_be(*((uint64_t *)prop->pr_values + 1));

  prop = dt_node_prop_extract(node, "interrupt-parent");
  if (prop == NULL) {
    return -KER_DTB_ER;
  }
  uint32_t intc = from_be(*((uint32_t *)prop->pr_values));
  irq_register_callback_t irq_register_callback;
  intc_get_irq_reg(intc, &irq_register_callback);

  if (irq_register_callback.cb_func == NULL) {
    info("intc %u not found, register %s to root intc 0\n", intc, node->nd_name);
    intc = 0;
    irq_register_callback.cb_func = intc_register_handler;
  }

  prop = dt_node_prop_extract(node, "interrupts");
  if (prop == NULL) {
    return -KER_DTB_ER;
  }
  uint32_t int_num = from_be(*((uint32_t *)prop->pr_values));

  struct sifiveuart0 *uart = kalloc(sizeof(struct sifiveuart0));
  uart->su_name = node->nd_name;
  uart->su_addr = addr;
  uart->su_size = size;
  uart->su_rd_buf.buf_data = palloc(SERIAL_BUFFER_SIZE, PGSIZE);
  uart->su_rd_buf.buf_off_b = 0;
  uart->su_rd_buf.buf_off_e = -1;
  uart->su_rd_buf.buf_size = 0;
  uart->su_wr_buf.buf_data = palloc(SERIAL_BUFFER_SIZE, PGSIZE);
  uart->su_wr_buf.buf_off_b = 0;
  uart->su_wr_buf.buf_off_e = -1;
  uart->su_wr_buf.buf_size = 0;

  info("%s probed, interrupt %08x registered to intc %u\n", node->nd_name, int_num, intc);
  struct fmt_mem_range mem_range = {.addr = addr, .size = size};
  info("\tlocates at %pM (size: %pB)\n", &mem_range, &size);
  cb_decl(trap_callback_t, trap_callback, sifiveuart0_handle_int, uart);
  catch_e(cb_invoke(irq_register_callback)(int_num, trap_callback));

  sifiveuart0_init(uart);
  cb_decl(flush_callback_t, flush_callback, sifiveuart0_flush, uart);
  cb_decl(putc_callback_t, putc_callback, sifiveuart0_putc, uart);
  cb_decl(getc_callback_t, getc_callback, sifiveuart0_getc, uart);
  serial_register_dev(node->nd_name, flush_callback, putc_callback, getc_callback);
  vmm_register_mmio(uart->su_name, &uart->su_addr, uart->su_size);

  return KER_SUCCESS;
}

static struct device sifiveuart0_device = {
    .d_pred = sifiveuart0_pred,
    .d_probe = sifiveuart0_probe,
};

device_init(sifiveuart0_device, low);

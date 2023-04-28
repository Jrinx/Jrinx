#include "uart16550a.h"
#include "serial.h"
#include <endian.h>
#include <kern/drivers/device.h>
#include <kern/drivers/intc.h>
#include <kern/drivers/serialport.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lib/sync.h>
#include <kern/mm/kalloc.h>
#include <kern/mm/vmm.h>
#include <lib/string.h>

struct uart16550a {
  char *ur_name;
  uintptr_t ur_addr;
  uintmax_t ur_size;
  uint32_t ur_shift;
  struct {
    uint8_t *buf_data;
    size_t buf_off_b;
    size_t buf_off_e;
    size_t buf_size;
  } ur_rd_buf, ur_wr_buf;
};

static inline void uart16550a_write(struct uart16550a *uart, unsigned long addr, uint8_t data) {
  fence(w, o);
  *((volatile uint8_t *)(uart->ur_addr + addr * (1 << uart->ur_shift))) = data;
}

static inline uint8_t uart16550a_read(struct uart16550a *uart, unsigned long addr) {
  uint8_t c = *((volatile uint8_t *)(uart->ur_addr + addr * (1 << uart->ur_shift)));
  fence(i, r);
  return c;
}

static void uart16550a_init(struct uart16550a *uart) {
  uart16550a_write(uart, COM_FCR, 0);                      // disable fifo
  uart16550a_write(uart, COM_LCR, 0b11);                   // baud rate 8n1
  uart16550a_write(uart, COM_MCR, 0);                      // disable modem
  uart16550a_write(uart, COM_IER, COM_IER_RCV_DATA_AVAIL); // enable receive data interrupt
}

static int uart16550a_getc(void *ctx, uint8_t *c) {
  struct uart16550a *uart = ctx;
  if (uart->ur_rd_buf.buf_size > 0) {
    uint8_t r = uart->ur_rd_buf.buf_data[uart->ur_rd_buf.buf_off_b];
    *c = r == 0xffU ? 0 : r;
    uart->ur_rd_buf.buf_off_b = (uart->ur_rd_buf.buf_off_b + 1) % uart->ur_rd_buf.buf_size;
    uart->ur_rd_buf.buf_size--;
    return 1;
  }
  if ((uart16550a_read(uart, COM_LSR) & COM_LSR_DATA_AVAIL) == 0) {
    return 0;
  }
  uint8_t r = uart16550a_read(uart, COM_RBR);
  *c = r == 0xffU ? 0 : r;
  return 1;
}

static int uart16550a_putc(void *ctx, uint8_t c) {
  struct uart16550a *uart = ctx;
  if (uart->ur_wr_buf.buf_size < SERIAL_BUFFER_SIZE) {
    uart->ur_wr_buf.buf_off_e = (uart->ur_wr_buf.buf_off_e + 1) % SERIAL_BUFFER_SIZE;
    uart->ur_wr_buf.buf_data[uart->ur_wr_buf.buf_off_e] = c;
    uart->ur_wr_buf.buf_size++;
    uart16550a_write(uart, COM_IER, COM_IER_RCV_DATA_AVAIL | COM_IER_TRANS_HOLD_REG_EMPTY);
    return 1;
  }
  return 0;
}

static void uart16550a_flush(void *ctx) {
  struct uart16550a *uart = ctx;
  while (uart->ur_wr_buf.buf_size > 0) {
    while ((uart16550a_read(uart, COM_LSR) & COM_LSR_THR_EMPTY) == 0) {
    }
    uart16550a_write(uart, COM_THR, uart->ur_wr_buf.buf_data[uart->ur_wr_buf.buf_off_b]);
    uart->ur_wr_buf.buf_off_b = (uart->ur_wr_buf.buf_off_b + 1) % SERIAL_BUFFER_SIZE;
    uart->ur_wr_buf.buf_size--;
  }
}

static long uart16550a_handle_int(void *ctx, unsigned long trap_num) {
  struct uart16550a *uart = ctx;
  uint8_t iir = uart16550a_read(uart, COM_IIR);
  switch (iir & COM_IIR_CAUSE) {
  case 0b0100:
  case 0b1100:
    while ((uart16550a_read(uart, COM_LSR) & COM_LSR_DATA_AVAIL) != 0) {
      if (uart->ur_rd_buf.buf_size == SERIAL_BUFFER_SIZE) {
        break;
      }
      uart->ur_rd_buf.buf_off_e = (uart->ur_rd_buf.buf_off_e + 1) % SERIAL_BUFFER_SIZE;
      uart->ur_rd_buf.buf_data[uart->ur_rd_buf.buf_off_e] = uart16550a_read(uart, COM_RBR);
      uart->ur_rd_buf.buf_size++;
    }
    break;
  case 0b0010:
    while ((uart16550a_read(uart, COM_LSR) & COM_LSR_THR_EMPTY) != 0) {
      if (uart->ur_wr_buf.buf_size == 0) {
        break;
      }
      uart16550a_write(uart, COM_THR, uart->ur_wr_buf.buf_data[uart->ur_wr_buf.buf_off_b]);
      uart->ur_wr_buf.buf_off_b = (uart->ur_wr_buf.buf_off_b + 1) % SERIAL_BUFFER_SIZE;
      uart->ur_wr_buf.buf_size--;
    }
    if (uart->ur_wr_buf.buf_size == 0) {
      uart16550a_write(uart, COM_IER, COM_IER_RCV_DATA_AVAIL);
    } else {
      uart16550a_write(uart, COM_IER, COM_IER_RCV_DATA_AVAIL | COM_IER_TRANS_HOLD_REG_EMPTY);
    }
    break;
  default:
    fatal("unexpected interrupt cause %u\n", iir & COM_IIR_CAUSE);
  }
  return KER_SUCCESS;
}

static int uart16550a_pred(const struct dev_node *node) {
  struct dev_node_prop *prop = dt_node_prop_extract(node, "compatible");
  return prop != NULL && dt_match_strlist(prop->pr_values, prop->pr_len, "ns16550a");
}

static long uart16550a_probe(const struct dev_node *node) {
  struct dev_node_prop *prop;
  prop = dt_node_prop_extract(node, "reg");
  if (prop == NULL) {
    return -KER_DTB_ER;
  }
  uint64_t addr = from_be(*((uint64_t *)prop->pr_values));
  uint64_t size = from_be(*((uint64_t *)prop->pr_values + 1));

  uint32_t shift = 0;
  prop = dt_node_prop_extract(node, "reg-shift");
  if (prop != NULL) {
    shift = from_be(*((uint32_t *)prop->pr_values));
  }

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

  struct uart16550a *uart = kalloc(sizeof(struct uart16550a));
  uart->ur_name = node->nd_name;
  uart->ur_addr = addr;
  uart->ur_size = size;
  uart->ur_shift = shift;
  uart->ur_rd_buf.buf_data = kalloc(SERIAL_BUFFER_SIZE);
  uart->ur_rd_buf.buf_off_b = 0;
  uart->ur_rd_buf.buf_off_e = -1;
  uart->ur_rd_buf.buf_size = 0;
  uart->ur_wr_buf.buf_data = kalloc(SERIAL_BUFFER_SIZE);
  uart->ur_wr_buf.buf_off_b = 0;
  uart->ur_wr_buf.buf_off_e = -1;
  uart->ur_wr_buf.buf_size = 0;

  info("%s probed (shift: %u), interrupt %08x registered to intc %u\n", node->nd_name, shift,
       int_num, intc);
  struct fmt_mem_range mem_range = {.addr = addr, .size = size};
  info("\tlocates at %pM (size: %pB)\n", &mem_range, &size);
  cb_decl(trap_callback_t, trap_callback, uart16550a_handle_int, uart);
  catch_e(cb_invoke(irq_register_callback)(int_num, trap_callback));

  uart16550a_init(uart);
  cb_decl(flush_callback_t, flush_callback, uart16550a_flush, uart);
  cb_decl(putc_callback_t, putc_callback, uart16550a_putc, uart);
  cb_decl(getc_callback_t, getc_callback, uart16550a_getc, uart);
  serial_register_dev(node->nd_name, flush_callback, putc_callback, getc_callback);
  vmm_register_mmio(uart->ur_name, &uart->ur_addr, uart->ur_size);

  return KER_SUCCESS;
}

static struct device uart16550a_device = {
    .d_pred = uart16550a_pred,
    .d_probe = uart16550a_probe,
};

device_init(uart16550a_device, low);

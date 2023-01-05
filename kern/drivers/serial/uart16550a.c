#include <endian.h>
#include <kern/drivers/device.h>
#include <kern/drivers/intc.h>
#include <kern/drivers/serial/uart16550a.h>
#include <kern/drivers/serialport.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lib/sync.h>
#include <kern/mm/pmm.h>
#include <kern/mm/vmm.h>
#include <layouts.h>
#include <lib/string.h>

static unsigned long uart16550a_addr;
static unsigned long uart16550a_size;
static unsigned long uart16550a_shift = 0;

static inline void uart16550a_write(unsigned long addr, uint8_t data) {
  fence(w, o);
  *((volatile uint8_t *)(uart16550a_addr + addr * (1 << uart16550a_shift))) = data;
}

static inline uint8_t uart16550a_read(unsigned long addr) {
  uint8_t c = *((volatile uint8_t *)(uart16550a_addr + addr * (1 << uart16550a_shift)));
  fence(i, r);
  return c;
}

static void uart16550a_init(void) {
  uart16550a_write(COM_FCR, 0);                      // disable fifo
  uart16550a_write(COM_LCR, 0b11);                   // baud rate 8n1
  uart16550a_write(COM_MCR, 0);                      // disable modem
  uart16550a_write(COM_IER, COM_IER_RCV_DATA_AVAIL); // enable receive data interrupt
}

static int uart16550a_getc(uint8_t *c) {
  if ((uart16550a_read(COM_LSR) & COM_LSR_DATA_AVAIL) == 0) {
    return 0;
  }
  uint8_t r = uart16550a_read(COM_RBR);
  *c = r == 0xffU ? 0 : r;
  return 1;
}

static int uart16550a_putc(uint8_t c) {
  if ((uart16550a_read(COM_LSR) & COM_LSR_THR_EMPTY) == 0) {
    return 0;
  }
  uart16550a_write(COM_THR, c);
  return 1;
}

static long uart16550a_setup_map() {
  vaddr_t va = {.val = uart16550a_addr + DEVOFFSET};
  paddr_t pa = {.val = uart16550a_addr};
  perm_t perm = {.bits = {.a = 1, .d = 1, .r = 1, .w = 1, .g = 1}};
  info("set up uart16550a mapping at ");
  mem_print_range(uart16550a_addr + DEVOFFSET, uart16550a_size, NULL);
  for (; va.val < uart16550a_addr + DEVOFFSET + uart16550a_size;
       va.val += PGSIZE, pa.val += PGSIZE) {
    catch_e(pt_map(kern_pgdir, va, pa, perm));
  }
  uart16550a_addr += DEVOFFSET;
  return KER_SUCCESS;
}

static long uart16550a_handle_int(unsigned long trap_num) {
  // TODO: notify those processes waiting uart input
  return KER_SUCCESS;
}

static long uart16550a_probe(const struct dev_node *node) {
  struct dev_node_prop *prop;
  prop = dt_node_prop_extract(node, "compatible");
  if (prop == NULL || !dt_match_strlist(prop->pr_values, prop->pr_len, "ns16550a")) {
    return KER_SUCCESS;
  }

  prop = dt_node_prop_extract(node, "reg");
  if (prop == NULL) {
    return -KER_DTB_ER;
  }
  uart16550a_addr = from_be(*((uint64_t *)prop->pr_values));
  uart16550a_size = from_be(*((uint64_t *)prop->pr_values + 1));
  vmm_register_mmio(uart16550a_setup_map);

  prop = dt_node_prop_extract(node, "reg-shift");
  if (prop != NULL) {
    uart16550a_shift = from_be(*((uint32_t *)prop->pr_values));
  }

  prop = dt_node_prop_extract(node, "interrupt-parent");
  if (prop == NULL) {
    return -KER_DTB_ER;
  }
  uint32_t intc = from_be(*((uint32_t *)prop->pr_values));
  irq_phandle_t phandle = intc_get_phandle(intc);

  if (phandle == NULL) {
    info("intc %08x not found, register %s to root intc\n", intc, node->nd_name);
    intc = 0;
    phandle = intc_register_handler;
  }

  prop = dt_node_prop_extract(node, "interrupts");
  if (prop == NULL) {
    return -KER_DTB_ER;
  }
  uint32_t int_num = from_be(*((uint32_t *)prop->pr_values));

  info("%s probed, interrupt %08x registered to intc %08x\n", node->nd_name, int_num, intc);
  catch_e(phandle(int_num, uart16550a_handle_int));

  uart16550a_init();
  serial_register_dev(node->nd_name, uart16550a_putc, uart16550a_getc);

  return KER_SUCCESS;
}

struct device uart16550a_device = {
    .d_probe = uart16550a_probe,
    .d_probe_pri = LOWEST,
};

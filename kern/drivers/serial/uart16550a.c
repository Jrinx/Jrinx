#include <endian.h>
#include <kern/drivers/device.h>
#include <kern/drivers/intc.h>
#include <kern/drivers/serial/uart16550a.h>
#include <kern/drivers/serialport.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lib/sync.h>
#include <kern/lock/lock.h>
#include <kern/lock/spinlock.h>
#include <kern/mm/pmm.h>
#include <kern/mm/vmm.h>
#include <layouts.h>
#include <lib/string.h>

struct uart16550a {
  char *ur_name;
  unsigned long ur_addr;
  unsigned long ur_size;
  uint32_t ur_shift;
  struct lock spinlock_of(ur);
  LIST_ENTRY(uart16550a) ur_link;
};

static LIST_HEAD(, uart16550a) uart16550a_list;

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
  if ((uart16550a_read(uart, COM_LSR) & COM_LSR_DATA_AVAIL) == 0) {
    return 0;
  }
  uint8_t r = uart16550a_read(uart, COM_RBR);
  *c = r == 0xffU ? 0 : r;
  return 1;
}

static int uart16550a_putc(void *ctx, uint8_t c) {
  struct uart16550a *uart = ctx;
  if ((uart16550a_read(uart, COM_LSR) & COM_LSR_THR_EMPTY) == 0) {
    return 0;
  }
  uart16550a_write(uart, COM_THR, c);
  return 1;
}

static long uart16550a_setup_map(void *ctx) {
  struct uart16550a *uart = ctx;
  unsigned long addr = uart->ur_addr;
  unsigned long size = uart->ur_size;
  vaddr_t va = {.val = addr + DEVOFFSET};
  paddr_t pa = {.val = addr};
  perm_t perm = {.bits = {.a = 1, .d = 1, .r = 1, .w = 1, .g = 1}};
  info("set up %s mapping at ", uart->ur_name);
  mem_print_range(addr + DEVOFFSET, size, NULL);
  for (; va.val < addr + DEVOFFSET + size; va.val += PGSIZE, pa.val += PGSIZE) {
    catch_e(pt_map(kern_pgdir, va, pa, perm));
  }
  uart->ur_addr += DEVOFFSET;
  return KER_SUCCESS;
}

static long uart16550a_handle_int(void *ctx, unsigned long trap_num) {
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
  unsigned long addr = from_be(*((uint64_t *)prop->pr_values));
  unsigned long size = from_be(*((uint64_t *)prop->pr_values + 1));

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
  void *parent_ctx;
  irq_phandle_t phandle = intc_get_phandle(intc, &parent_ctx);

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

  struct uart16550a *uart = alloc(sizeof(struct uart16550a), sizeof(struct uart16550a));
  uart->ur_name = node->nd_name;
  uart->ur_addr = addr;
  uart->ur_size = size;
  uart->ur_shift = shift;
  LIST_INSERT_HEAD(&uart16550a_list, uart, ur_link);

  info("%s probed (shift: %08x), interrupt %08x registered to intc %08x\n", node->nd_name,
       shift, int_num, intc);
  info("\tlocates at ");
  mem_print_range(addr, size, NULL);
  catch_e(phandle(parent_ctx, int_num, uart16550a_handle_int, uart));

  uart16550a_init(uart);
  serial_register_dev(node->nd_name, uart16550a_putc, uart16550a_getc, uart);
  vmm_register_mmio(uart16550a_setup_map, uart);

  return KER_SUCCESS;
}

struct device uart16550a_device = {
    .d_probe = uart16550a_probe,
    .d_probe_pri = LOWEST,
};

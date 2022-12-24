#include <kern/lib/debug.h>
#include <kern/lib/logger.h>
#include <kern/lib/sbi.h>
#include <layouts.h>
#include <stdint.h>

void kernel_init(unsigned long hartid, unsigned long opaque) {
  static int is_master = 1;
  static unsigned long hart_table = 0;
  if (is_master) {
    info("[ hart %ld ] Hello Jrinx, I am master hart!\n", hartid);
    is_master = 0;
    for (int i = 0; i < CONFIG_NR_CORES; i++) {
      if (i != hartid) {
        hart_table |= 1 << i;
        panic_e(sbi_hart_start(i, KERNBASE, 0));
      }
    }
    panic_e(sbi_send_ipi(&hart_table));
    while (hart_table) {
    }
    haltk("[ hart %ld ] all cores running!", hartid);
  } else {
    while ((hart_table & ((1 << hartid) - 1)) != 0) {
    }
    info("[ hart %ld ] Hello Jrinx, I am slave hart!\n", hartid);
    hart_table &= ~(1 << hartid);
    while (1) {
    }
  }
}

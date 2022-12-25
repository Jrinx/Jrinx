#include <kern/lib/debug.h>
#include <kern/lib/logger.h>
#include <kern/lib/sbi.h>
#include <kern/lock/lock.h>
#include <kern/lock/spinlock.h>
#include <layouts.h>
#include <stdint.h>

void kernel_init(unsigned long hartid, unsigned long opaque) {
  static int is_master = 1;
  static volatile unsigned long init_cnt = 0;
  static with_spinlock(init_cnt);
  hrt_set_id(hartid);
  if (is_master) {
    lk_init();

    info("[ hart %ld ] Hello Jrinx, I am master hart!\n", hartid);

    init_cnt++;
    is_master = 0;
    unsigned long hart_table = 0;
    for (int i = 0; i < CONFIG_NR_CORES; i++) {
      if (i != hartid) {
        hart_table |= 1 << i;
        panic_e(sbi_hart_start(i, KERNBASE, 0));
      }
    }
    panic_e(sbi_send_ipi(&hart_table));
    while (init_cnt < CONFIG_NR_CORES) {
    }
    haltk("[ hart %ld ] all cores are running, halt!", hartid);
  } else {
    info("[ hart %ld ] Hello Jrinx, I am slave hart!\n", hartid);
    panic_e(lk_acquire(&spinlock_of(init_cnt)));
    init_cnt++;
    panic_e(lk_release(&spinlock_of(init_cnt)));
    while (1) {
    }
  }
}

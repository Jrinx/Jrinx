#include <kern/drivers/cpus.h>
#include <kern/drivers/device.h>
#include <kern/drivers/devicetree.h>
#include <kern/lib/debug.h>
#include <kern/lib/logger.h>
#include <kern/lib/regs.h>
#include <kern/lib/sync.h>
#include <kern/lock/lock.h>
#include <kern/lock/spinlock.h>
#include <kern/mm/pmm.h>
#include <kern/mm/vmm.h>
#include <layouts.h>
#include <lib/string.h>
#include <stddef.h>
#include <stdint.h>

union kern_init_arg {
  void *dtb_addr;         // master
  unsigned long stacktop; // slave
};

struct dev_tree boot_dt;

void kernel_init(unsigned long hartid, union kern_init_arg arg) {
  static volatile unsigned long init_state = 0;
  static with_spinlock(init_state);
  hrt_set_id(hartid);
  if (cpus_stacktop == NULL) {
    lk_init();

    info("Hello Jrinx, I am master hart!\n");

    panic_e(dt_load(arg.dtb_addr, &boot_dt));
    panic_e(device_init());
    panic_e(device_probe(&boot_dt));
    pmm_init();
    vmm_setup_kern();
    vmm_start();

    panic_e(lk_acquire(&spinlock_of(init_state)));
    init_state++;
    panic_e(lk_release(&spinlock_of(init_state)));
    while (init_state != cpus_get_count()) {
    }
    halt("All cores are running, halt!\n");
  } else {
    while (init_state == 0) {
    }
    vmm_start();
    info("Hello Jrinx, I am slave hart!\n");
    panic_e(lk_acquire(&spinlock_of(init_state)));
    init_state++;
    panic_e(lk_release(&spinlock_of(init_state)));
    while (1) {
    }
  }
}

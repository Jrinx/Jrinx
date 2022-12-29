#include <kern/drivers/cpus.h>
#include <kern/drivers/device.h>
#include <kern/drivers/devicetree.h>
#include <kern/lib/debug.h>
#include <kern/lib/logger.h>
#include <kern/lib/regs.h>
#include <kern/lib/sync.h>
#include <kern/lock/lock.h>
#include <kern/lock/spinlock.h>
#include <kern/mm/mem.h>
#include <kern/mm/vm.h>
#include <layouts.h>
#include <lib/string.h>
#include <stddef.h>
#include <stdint.h>

union kern_init_arg {
  void *dtb_addr;         // master
  unsigned long stacktop; // slave
};

void kernel_init(unsigned long hartid, union kern_init_arg arg) {
  static volatile unsigned long init_state = 0;
  static with_spinlock(init_state);
  hrt_set_id(hartid);
  if (cpus_stacktop == NULL) {
    lk_init();

    info("Hello Jrinx, I am master hart!\n");

    struct dev_tree dt;
    panic_e(dt_load(arg.dtb_addr, &dt));
    panic_e(device_init());
    panic_e(device_probe(&dt));
    memory_init();
    vm_init_kern_pgdir();
    vm_start();

    panic_e(lk_acquire(&spinlock_of(init_state)));
    init_state++;
    panic_e(lk_release(&spinlock_of(init_state)));
    while (init_state != cpus_get_count()) {
    }
    halt("All cores are running, halt!\n");
  } else {
    while (init_state == 0) {
    }
    vm_start();
    info("Hello Jrinx, I am slave hart!\n");
    panic_e(lk_acquire(&spinlock_of(init_state)));
    init_state++;
    panic_e(lk_release(&spinlock_of(init_state)));
    while (1) {
    }
  }
}

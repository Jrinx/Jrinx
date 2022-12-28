#include <kern/drivers/cpu.h>
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

void kernel_init(unsigned long hartid, void *dtb_addr) {
  static volatile unsigned long init_state = 0;
  static with_spinlock(init_state);
  hrt_set_id(hartid);
  if (!cpu_master_init) {
    cpu_master_init = 1;
    lk_init();

    info("[ hart %ld ] Hello Jrinx, I am master hart!\n", hartid);

    struct dev_tree dt;
    panic_e(dt_load(dtb_addr, &dt));
    panic_e(device_init());
    panic_e(device_probe(&dt));
    memory_init();
    vm_init();
    vm_start();

    panic_e(lk_acquire(&spinlock_of(init_state)));
    init_state++;
    panic_e(lk_release(&spinlock_of(init_state)));
    while (init_state != cpu_get_count()) {
    }
    haltk("[ hart %ld ] all cores are running, halt!", hartid);
  } else {
    while (init_state == 0) {
    }
    vm_start();
    info("[ hart %ld ] Hello Jrinx, I am slave hart!\n", hartid);
    panic_e(lk_acquire(&spinlock_of(init_state)));
    init_state++;
    panic_e(lk_release(&spinlock_of(init_state)));
    while (1) {
    }
  }
}

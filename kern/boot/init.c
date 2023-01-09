#include <kern/drivers/chosen.h>
#include <kern/drivers/cpus.h>
#include <kern/drivers/device.h>
#include <kern/drivers/devicetree.h>
#include <kern/drivers/serialport.h>
#include <kern/lib/debug.h>
#include <kern/lib/logger.h>
#include <kern/lib/regs.h>
#include <kern/lib/sbi.h>
#include <kern/lib/sync.h>
#include <kern/lock/lock.h>
#include <kern/lock/spinlock.h>
#include <kern/mm/pmm.h>
#include <kern/mm/vmm.h>
#include <layouts.h>
#include <lib/string.h>
#include <stddef.h>
#include <stdint.h>

struct dev_tree boot_dt;

static void print_boot_info(void) {
  printk("\nJrinx OS (revision: %s)\n", CONFIG_REVISON);
  static const char kernel_logo[] = CONFIG_JRINX_LOGO;
  for (size_t i = 0; i < sizeof(kernel_logo); i++) {
    long ret __attribute__((unused)) = sbi_console_putchar(kernel_logo[i]);
  }
}

void kernel_init(unsigned long hartid, void *dtb_addr) {
  static volatile unsigned long init_state = 0;
  static with_spinlock(init_state);
  hrt_set_id(hartid);
  if (cpus_stacktop == NULL) {
    print_boot_info();

    lk_init();
    info("Hello Jrinx, I am master hart!\n");

    panic_e(dt_load(dtb_addr, &boot_dt));
    panic_e(device_init());
    panic_e(device_probe(&boot_dt));
    panic_e(chosen_select_dev());

    pmm_init();
    vmm_setup_mmio();
    vmm_setup_kern();
    vmm_start();

    log_localize_output();

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

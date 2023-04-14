#include <kern/drivers/chosen.h>
#include <kern/drivers/cpus.h>
#include <kern/drivers/device.h>
#include <kern/drivers/devicetree.h>
#include <kern/drivers/serialport.h>
#include <kern/lib/bootargs.h>
#include <kern/lib/debug.h>
#include <kern/lib/logger.h>
#include <kern/lib/regs.h>
#include <kern/lib/sbi.h>
#include <kern/lib/sync.h>
#include <kern/lock/lock.h>
#include <kern/lock/spinlock.h>
#include <kern/mm/asid.h>
#include <kern/mm/kalloc.h>
#include <kern/mm/vmm.h>
#include <kern/multitask/sched.h>
#include <kern/traps/timer.h>
#include <kern/traps/traps.h>
#include <layouts.h>
#include <lib/string.h>
#include <stddef.h>
#include <stdint.h>

struct dev_tree boot_dt;
const char *boot_dt_model;

static void print_boot_info(void) {
  printk("\nJrinx OS (revision: %s)\n", CONFIG_REVISON);
  static const char kernel_logo[] = CONFIG_JRINX_LOGO;
  for (size_t i = 0; i < sizeof(kernel_logo); i++) {
    long ret __attribute__((unused)) = sbi_console_putchar(kernel_logo[i]);
  }
}

static void kernel_gen_init(void) {
  static volatile unsigned long gen_init_state = 0;
  static with_spinlock(gen_init_state);
  switch (hrt_get_id()) {
  case SYSCORE:
    panic_e(args_action());
  default:
    panic_e(lk_acquire(&spinlock_of(gen_init_state)));
    gen_init_state++;
    panic_e(lk_release(&spinlock_of(gen_init_state)));
    break;
  }
  trap_init_vec();
  while (gen_init_state != cpus_get_valid_count()) {
  }
}

static void kernel_sched(void) {
  static volatile unsigned long sched_state = 0;
  static with_spinlock(sched_state);
  switch (hrt_get_id()) {
  case SYSCORE:
    sched_global();
    break;
  default:
    // TODO: sched in smp
    break;
  }
  panic_e(lk_acquire(&spinlock_of(sched_state)));
  sched_state++;
  panic_e(lk_release(&spinlock_of(sched_state)));
  while (sched_state != cpus_get_valid_count()) {
  }
}

void __attribute__((noreturn)) kernel_init(unsigned long hartid, void *dtb_addr) {
  static volatile unsigned long init_state = 0;
  static with_spinlock(init_state);
  hrt_set_id(hartid);
  if (cpus_stacktop == NULL) {
    print_boot_info();

    info("Hello Jrinx, I am master hart!\n");

    kalloc_init();
    panic_e(dt_load(dtb_addr, &boot_dt));
    boot_dt_model = dt_get_model(&boot_dt);
    panic_e(device_probe(&boot_dt));
    panic_e(chosen_select_dev());
    panic_e(args_evaluate(chosen_get_bootargs()));

    asid_array_init();
    vmm_setup_mmio();
    vmm_setup_kern();
    vmm_start();
    vmm_summary();

    log_localize_output();
    time_event_init();
    traps_init();
    sched_init();
  } else {
    while (init_state == 0) {
    }
    vmm_start();
    info("Hello Jrinx, I am slave hart!\n");
  }
  panic_e(lk_acquire(&spinlock_of(init_state)));
  init_state++;
  panic_e(lk_release(&spinlock_of(init_state)));
  while (init_state != cpus_get_valid_count()) {
  }

  kernel_gen_init();
  kernel_sched();

  if (hrt_get_id() == SYSCORE) {
    halt("All cores are running, halt!\n");
  } else {
    while (1) {
    }
  }
}

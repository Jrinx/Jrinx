#include <kern/drivers/cpus.h>
#include <kern/drivers/serialport.h>
#include <kern/lib/boottime.h>
#include <kern/lib/debug.h>
#include <kern/lib/sbi.h>
#include <kern/lock/lock.h>
#include <kern/lock/spinlock.h>
#include <lib/printfmt.h>
#include <types.h>

static with_spinlock(print);

static void sbi_output(void *data, const char *buf, size_t len) {
  for (int i = 0; i < len; i++) {
    panic_e(sbi_console_putchar(buf[i]));
  }
}

static void serial_output(void *data, const char *buf, size_t len) {
  for (int i = 0; i < len; i++) {
    serial_blocked_putc(buf[i]);
  }
}

static cb_decl(fmt_callback_t, output_cb, sbi_output, NULL);

void log_localize_output(void) {
  output_cb.cb_func = serial_output;
  info("switch to local serial output\n");
}

void conslock_acquire(void) {
  panic_e(lk_acquire(&spinlock_of(print)));
}

void conslock_release(void) {
  panic_e(lk_release(&spinlock_of(print)));
}

void printk(const char *restrict fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vprintfmt(output_cb, fmt, ap);
  va_end(ap);
}

#define print_timestamp(color)                                                                 \
  ({                                                                                           \
    conslock_acquire();                                                                        \
    sys_time_t time = boottime_get_now();                                                      \
    if (time != 0) {                                                                           \
      printk(ANSI_COLOR_WRAP(color, "[ %pT hart#%lu ] %s:%lu <%s> "), &time, hrt_get_id(),     \
             file, lineno, func);                                                              \
    } else {                                                                                   \
      printk(ANSI_COLOR_WRAP(color, "[ ?.?????? hart#%lu ] %s:%lu <%s> "), hrt_get_id(), file, \
             lineno, func);                                                                    \
    }                                                                                          \
    va_list ap;                                                                                \
    va_start(ap, fmt);                                                                         \
    vprintfmt(output_cb, fmt, ap);                                                             \
    va_end(ap);                                                                                \
    conslock_release();                                                                        \
  })

void infok(const char *restrict file, unsigned long lineno, const char *restrict func,
           const char *restrict fmt, ...) {
  print_timestamp(ANSI_FG_GREEN);
}

void fatalk(const char *restrict file, unsigned long lineno, const char *restrict func,
            const char *restrict fmt, ...) {
  print_timestamp(ANSI_FG_RED);
  conslock_acquire();
  serial_flush();
  conslock_release();
  struct sbiret ret =
      sbi_system_reset(SBI_SRST_RESET_TYPE_WARM_REBOOT, SBI_SRST_RESET_REASON_SYSFAIL);
  info("shutdown failed: (%ld, %ld)\n", ret.error, ret.value);
  conslock_acquire();
  serial_flush();
  conslock_release();
  while (1) {
  }
}

void haltk(const char *restrict file, unsigned long lineno, const char *restrict func,
           const char *restrict fmt, ...) {
  print_timestamp(ANSI_FG_YELLOW);
  conslock_acquire();
  serial_flush();
  conslock_release();
  struct sbiret ret =
      sbi_system_reset(SBI_SRST_RESET_TYPE_WARM_REBOOT, SBI_SRST_RESET_REASON_NONE);
  info("shutdown failed: (%ld, %ld)\n", ret.error, ret.value);
  conslock_acquire();
  serial_flush();
  conslock_release();
  while (1) {
  }
}

#undef print_timestamp

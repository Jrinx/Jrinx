#include <kern/lib/debug.h>
#include <kern/lib/sbi.h>
#include <lib/printfmt.h>

static void outputk(void *data, const char *buf, size_t len) {
  for (int i = 0; i < len; i++) {
    panic_e(sbi_console_putchar(buf[i]));
  }
}

void printk(const char *restrict fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vprintfmt(outputk, NULL, fmt, ap);
  va_end(ap);
}

void panick(const char *restrict fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vprintfmt(outputk, NULL, fmt, ap);
  va_end(ap);
  struct sbiret ret =
      sbi_system_reset(SBI_SRST_RESET_TYPE_SHUTDOWN, SBI_SRST_RESET_REASON_SYSFAIL);
  info("shutdown failed: (%ld, %ld)\n", ret.error, ret.value);
  while (1) {
  }
}

void haltk(const char *restrict fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vprintfmt(outputk, NULL, fmt, ap);
  va_end(ap);
  sbi_shutdown();
}

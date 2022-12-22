#include <kern/sbi/ecall-intf.h>
#include <lib/printfmt.h>

static void outputk(void *data, const char *buf, size_t len) {
  for (int i = 0; i < len; i++) {
    sbi_console_putchar(buf[i]);
  }
}

void printk(const char *restrict fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vprintfmt(outputk, NULL, fmt, ap);
  va_end(ap);
}

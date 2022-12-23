#include <kern/lib/sbi.h>
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

void panick(const char *restrict file, const unsigned long line, const char *restrict fmt,
            ...) {
  printk("%s:%ld ", file, line);
  va_list ap;
  va_start(ap, fmt);
  vprintfmt(outputk, NULL, fmt, ap);
  va_end(ap);
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

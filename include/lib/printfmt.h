#ifndef _LIB_PRINTFMT_H_
#define _LIB_PRINTFMT_H_

#include <callback.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

typedef void (*fmt_callback_func_t)(void *, const char *, size_t);

typedef cb_typedef(fmt_callback_func_t) fmt_callback_t;

struct fmt_mem_range { // %pM
  unsigned long addr;
  size_t size;
};

void vprintfmt(fmt_callback_t out, const char *restrict fmt, va_list ap);

#endif

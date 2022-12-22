#ifndef _LIB_PRINTFMT_H_
#define _LIB_PRINTFMT_H_

#include <stdarg.h>
#include <types.h>

typedef void (*fmt_callback_t)(void *, const char *, size_t);

void vprintfmt(fmt_callback_t out, void *data, const char *restrict fmt, va_list ap);

#endif

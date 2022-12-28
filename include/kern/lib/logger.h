#ifndef _KERN_LIB_LOGGER_H_
#define _KERN_LIB_LOGGER_H_

#include <kern/lib/hart.h>
#include <stdarg.h>

void printk(const char *restrict fmt, ...) __attribute__((format(printf, 1, 2)));
void panick(const char *restrict fmt, ...) __attribute__((format(printf, 1, 2), noreturn));
void haltk(const char *restrict fmt, ...) __attribute__((noreturn));

#define _log_args(msg, ...)                                                                    \
  "[ hart %ld ] %s:%d <%s> " msg, hrt_get_id(), __FILE__, __LINE__, __func__, ##__VA_ARGS__

#define info(msg, ...) printk(_log_args(msg, ##__VA_ARGS__))
#define fatal(msg, ...) panick(_log_args(msg, ##__VA_ARGS__))
#define halt(msg, ...) haltk(_log_args(msg, ##__VA_ARGS__))

#endif

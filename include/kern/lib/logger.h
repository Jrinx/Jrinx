#ifndef _KERN_LIB_LOGGER_H_
#define _KERN_LIB_LOGGER_H_

#include <ansictl.h>
#include <kern/lib/hart.h>
#include <stdarg.h>

void log_switch_to_local_serial_output(void);
void printk(const char *restrict fmt, ...) __attribute__((format(printf, 1, 2)));
void panick(const char *restrict fmt, ...) __attribute__((format(printf, 1, 2), noreturn));
void haltk(const char *restrict fmt, ...) __attribute__((noreturn));

#define _log_args(color, msg, ...)                                                             \
  ANSI_COLOR_WRAP(color, "[ hart %ld ] %s:%d <%s> ")                                           \
  msg, hrt_get_id(), __FILE__, __LINE__, __func__, ##__VA_ARGS__

#define info(msg, ...) printk(_log_args(ANSI_FG_GREEN, msg, ##__VA_ARGS__))
#define fatal(msg, ...) panick(_log_args(ANSI_FG_RED, msg, ##__VA_ARGS__))
#define halt(msg, ...) haltk(_log_args(ANSI_FG_YELLOW, msg, ##__VA_ARGS__))

#endif

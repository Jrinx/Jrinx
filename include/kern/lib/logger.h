#ifndef _KERN_LIB_LOGGER_H_
#define _KERN_LIB_LOGGER_H_

#include <ansictl.h>
#include <kern/drivers/realtime.h>
#include <kern/lib/hart.h>
#include <stdarg.h>

void log_localize_output(void);
void conslock_acquire(void);
void conslock_release(void);
void printk(const char *restrict fmt, ...) __attribute__((format(printf, 1, 2)));
void panick(const char *restrict fmt, ...) __attribute__((format(printf, 1, 2), noreturn));
void haltk(const char *restrict fmt, ...) __attribute__((noreturn));

#define _log_prefix "[ %3lu.%03lu hart#%ld ] %s:%d <%s> "

#define _log_args(...) sec, millisec, hrt_get_id(), __FILE__, __LINE__, __func__, ##__VA_ARGS__

#define info(msg, ...)                                                                         \
  ({                                                                                           \
    uint64_t sec;                                                                              \
    uint64_t millisec;                                                                         \
    conslock_acquire();                                                                        \
    realtime_read_boot_time_sec_millisec(&sec, &millisec);                                     \
    printk(ANSI_COLOR_WRAP(ANSI_FG_GREEN, _log_prefix) msg, _log_args(__VA_ARGS__));           \
    conslock_release();                                                                        \
  })

#define fatal(msg, ...)                                                                        \
  ({                                                                                           \
    uint64_t sec;                                                                              \
    uint64_t millisec;                                                                         \
    realtime_read_boot_time_sec_millisec(&sec, &millisec);                                     \
    panick(ANSI_COLOR_WRAP(ANSI_FG_RED, _log_prefix) msg, _log_args(__VA_ARGS__));             \
  })

#define halt(msg, ...)                                                                         \
  ({                                                                                           \
    uint64_t sec;                                                                              \
    uint64_t millisec;                                                                         \
    realtime_read_boot_time_sec_millisec(&sec, &millisec);                                     \
    haltk(ANSI_COLOR_WRAP(ANSI_FG_YELLOW, _log_prefix) msg, _log_args(__VA_ARGS__));           \
  })

#endif

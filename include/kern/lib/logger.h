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

void infok(const char *restrict file, unsigned long lineno, const char *restrict func,
           const char *restrict fmt, ...);

void fatalk(const char *restrict file, unsigned long lineno, const char *restrict func,
            const char *restrict fmt, ...) __attribute__((noreturn));

void haltk(const char *restrict file, unsigned long lineno, const char *restrict func,
           const char *restrict fmt, ...) __attribute__((noreturn));

#define info(msg, ...) infok(__FILE__, __LINE__, __func__, msg, ##__VA_ARGS__)
#define fatal(msg, ...) fatalk(__FILE__, __LINE__, __func__, msg, ##__VA_ARGS__)
#define halt(msg, ...) haltk(__FILE__, __LINE__, __func__, msg, ##__VA_ARGS__)

#endif

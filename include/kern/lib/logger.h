#ifndef _KERN_LIB_LOGGER_H_
#define _KERN_LIB_LOGGER_H_

#include <ansictl.h>
#include <kern/drivers/realtime.h>
#include <kern/lib/hart.h>
#include <lib/printfmt.h>
#include <stdarg.h>

void printk(const char *restrict fmt, ...) __attribute__((format(printf, 1, 2)));

void infok(const char *restrict file, unsigned long lineno, const char *restrict func,
           const char *restrict fmt, ...) __attribute__((format(printf, 4, 5)));

void fatalk(const char *restrict file, unsigned long lineno, const char *restrict func,
            const char *restrict fmt, ...) __attribute__((format(printf, 4, 5), noreturn));

void haltk(const char *restrict file, unsigned long lineno, const char *restrict func,
           const char *restrict fmt, ...) __attribute__((format(printf, 4, 5), noreturn));

#define info(msg, ...) infok(__FILE__, __LINE__, __func__, msg, ##__VA_ARGS__)
#define fatal(msg, ...) fatalk(__FILE__, __LINE__, __func__, msg, ##__VA_ARGS__)
#define halt(msg, ...) haltk(__FILE__, __LINE__, __func__, msg, ##__VA_ARGS__)

#endif

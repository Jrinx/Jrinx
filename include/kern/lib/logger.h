#ifndef _KERN_LIB_LOGGER_H_
#define _KERN_LIB_LOGGER_H_

#include <stdarg.h>

void printk(const char *restrict fmt, ...) __attribute__((format(printf, 1, 2)));
void panick(const char *restrict file, const unsigned long line, const char *restrict fmt, ...)
    __attribute__((format(printf, 3, 4)));
void haltk(const char *restrict fmt, ...) __attribute__((noreturn));

#define fatal(msg, ...) panick(__FILE__, __LINE__, msg, ##__VA_ARGS__)

#endif

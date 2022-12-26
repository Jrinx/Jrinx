#ifndef _KERN_LIB_LOGGER_H_
#define _KERN_LIB_LOGGER_H_

#include <stdarg.h>

void printk(const char *restrict fmt, ...) __attribute__((format(printf, 1, 2)));
void panick(const char *restrict fmt, ...) __attribute__((format(printf, 1, 2), noreturn));
void haltk(const char *restrict fmt, ...) __attribute__((noreturn));

#define info(msg, ...) printk("%s:%d " msg, __FILE__, __LINE__, ##__VA_ARGS__)
#define fatal(msg, ...) panick("%s:%d " msg, __FILE__, __LINE__, ##__VA_ARGS__)

#endif

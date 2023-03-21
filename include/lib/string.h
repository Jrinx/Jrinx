#ifndef _LIB_STRING_H_
#define _LIB_STRING_H_

#include <stdarg.h>
#include <stddef.h>

void *memcpy(void *restrict dst, const void *restrict src, size_t n);
void *memset(void *restrict dst, int c, size_t n);
size_t strlen(const char *s);
char *strcpy(char *restrict dst, const char *restrict src);
const char *strchr(const char *restrict s, int c);
int strcmp(const char *p, const char *q);
int atoi(const char *str);
int sprintf(char *buf, const char *restrict fmt, ...);

#endif

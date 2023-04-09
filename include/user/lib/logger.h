#ifndef _USER_LIB_LOGGER_H_
#define _USER_LIB_LOGGER_H_

#include <types.h>

void logger_init(PARTITION_ID_TYPE partition_id);
void printu(const char *restrict fmt, ...) __attribute__((format(printf, 1, 2)));
void infou(const char *restrict file, unsigned long lineno, const char *restrict func,
           const char *restrict fmt, ...) __attribute__((format(printf, 4, 5)));
void fatalu(const char *restrict file, unsigned long lineno, const char *restrict func,
            const char *restrict fmt, ...) __attribute__((format(printf, 4, 5), noreturn));
void haltu(const char *restrict file, unsigned long lineno, const char *restrict func,
           const char *restrict fmt, ...) __attribute__((format(printf, 4, 5), noreturn));

#define info(msg, ...) infou(__FILE__, __LINE__, __func__, msg, ##__VA_ARGS__)
#define fatal(msg, ...) fatalu(__FILE__, __LINE__, __func__, msg, ##__VA_ARGS__)
#define halt(msg, ...) haltu(__FILE__, __LINE__, __func__, msg, ##__VA_ARGS__)

#endif

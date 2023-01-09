#ifndef _KERN_DRIVERS_REALTIME_H_
#define _KERN_DRIVERS_REALTIME_H_

#include <callback.h>
#include <stdint.h>

typedef long (*read_time_func_t)(void *ctx, uint64_t *re);
typedef cb_typedef(read_time_func_t) read_time_callback_t;

void realtime_register_rtc(const char *name, read_time_callback_t read_time_callback);
int realtime_select_rtc(const char *name);
int realtime_read_time(uint64_t *re);

void realtime_read_boot_time_sec_millisec(uint64_t *sec, uint64_t *millisec);

#endif

#ifndef _KERN_DRIVERS_REALTIME_H_
#define _KERN_DRIVERS_REALTIME_H_

#include <callback.h>
#include <stdint.h>

typedef long (*read_time_func_t)(void *ctx, uint64_t *re);
typedef cb_typedef(read_time_func_t) read_time_callback_t;

void rt_register_dev(const char *name, read_time_callback_t read_time_callback);
int rt_select_dev(const char *name);
int rt_read_time(uint64_t *re);

int rt_read_boot_time_sec_msec(uint64_t *sec, uint64_t *millisec);

#endif

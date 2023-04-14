#ifndef _KERN_DRIVERS_CPUS_H_
#define _KERN_DRIVERS_CPUS_H_

#include <kern/drivers/device.h>
#include <stddef.h>

extern unsigned long *cpus_stacktop;
extern size_t *cpus_intp_layer;
extern int *cpus_retained_intp;

uint32_t cpus_get_timebase_freq(void);
unsigned long cpus_get_count(void);
unsigned long cpus_get_valid_count(void);

#endif

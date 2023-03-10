#ifndef _KERN_DRIVERS_CPUS_H_
#define _KERN_DRIVERS_CPUS_H_

#include <kern/drivers/device.h>

extern unsigned long *cpus_stacktop;

uint32_t cpus_get_timebase_freq(void);
unsigned long cpus_get_count(void);

#endif

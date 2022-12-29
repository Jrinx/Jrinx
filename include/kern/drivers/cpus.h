#ifndef _KERN_DRIVERS_CPUS_H_
#define _KERN_DRIVERS_CPUS_H_

extern struct device cpus_device;
extern unsigned long *cpus_stacktop;

unsigned long cpus_get_count(void);

#endif

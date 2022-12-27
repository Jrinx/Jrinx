#ifndef _KERN_DRIVERS_CPU_H_
#define _KERN_DRIVERS_CPU_H_

extern int cpu_master_init;
extern struct device cpu_device;
unsigned long cpu_get_count(void);

#endif

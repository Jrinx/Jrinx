#ifndef _KERN_DRIVERS_MEMS_H_
#define _KERN_DRIVERS_MEMS_H_

#include <kern/drivers/device.h>
#include <stddef.h>
#include <stdint.h>

size_t mem_get_num(void);
long mem_get_addr(unsigned i, uint64_t *addr);
long mem_get_size(unsigned i, uint64_t *size);

#endif

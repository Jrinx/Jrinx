#ifndef _KERN_MM_MEM_H_
#define _KERN_MM_MEM_H_

#include <stddef.h>

void *bare_alloc(size_t size);

extern struct device memory_device;

void memory_init(void);

#endif

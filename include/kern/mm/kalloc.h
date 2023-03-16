#ifndef _KERN_MM_KALLOC_H_
#define _KERN_MM_KALLOC_H_

#include <stddef.h>

void kalloc_init(void);
size_t kalloc_get_used(void);
void *kalloc(size_t size);
void kfree(void *ptr);

#endif

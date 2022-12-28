#ifndef _KERN_LIB_SYNC_H_
#define _KERN_LIB_SYNC_H_

#define fence(x, y) asm volatile("fence " #x ", " #y)
#define fence_i asm volatile("fence.i")
#define sfence_vma asm volatile("sfence.vma")

#endif

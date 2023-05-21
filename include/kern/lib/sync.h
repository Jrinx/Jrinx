#ifndef _KERN_LIB_SYNC_H_
#define _KERN_LIB_SYNC_H_

#include <attr.h>

#define fence(x, y) asm volatile("fence " #x ", " #y : : : "memory")
#define fence_i asm volatile("fence.i" : : : "memory")
#define sfence_vma asm volatile("sfence.vma x0, x0" : : : "memory")

__inline static inline void sfence_vma_va_asid(unsigned long va, unsigned long asid) {
  asm volatile("sfence.vma %0, %1" : : "r"(va), "r"(asid) : "memory");
}

__inline static inline void sfence_vma_asid(unsigned long asid) {
  asm volatile("sfence.vma x0, %0" : : "r"(asid) : "memory");
}

__inline static inline void sfence_vma_va(unsigned long va) {
  asm volatile("sfence.vma %0, x0" : : "r"(va) : "memory");
}

#endif

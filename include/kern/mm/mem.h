#ifndef _KERN_MM_MEM_H_
#define _KERN_MM_MEM_H_

#include <stddef.h>
#include <sys/queue.h>

struct phy_frame {
  unsigned long pf_ref;
  LIST_ENTRY(phy_frame) pf_link;
};

LIST_HEAD(phy_frame_list, phy_frame);

unsigned long mm_get_freemem_base(void);
void mm_print_range(unsigned long addr, unsigned long size, const char *suffix);

extern void *(*alloc)(size_t size, size_t align);
extern void (*free)(const void *ptr);

extern struct device memory_device;

void memory_init(void);
long pa2sel(unsigned long addr, unsigned long *sel) __attribute__((warn_unused_result));
long frame2sel(struct phy_frame *frame, unsigned long *sel) __attribute__((warn_unused_result));
long pa2frame(unsigned long addr, struct phy_frame **frame) __attribute__((warn_unused_result));
long frame2pa(struct phy_frame *frame, unsigned long *addr) __attribute__((warn_unused_result));
long phy_frame_alloc(struct phy_frame **frame) __attribute__((warn_unused_result));
long phy_frame_ref_dec(struct phy_frame *frame) __attribute__((warn_unused_result));
long phy_frame_ref_inc(struct phy_frame *frame) __attribute__((warn_unused_result));

#endif

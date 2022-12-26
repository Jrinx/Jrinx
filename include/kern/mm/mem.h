#ifndef _KERN_MM_MEM_H_
#define _KERN_MM_MEM_H_

#include <stddef.h>
#include <sys/queue.h>

struct phy_frame {
  unsigned long pf_ref;
  LIST_ENTRY(phy_frame) pf_link;
};

LIST_HEAD(phy_frame_list, phy_frame);

extern void *(*alloc)(size_t size);
extern void (*free)(void *ptr);

extern struct device memory_device;

void memory_init(void);
long pa2sel(unsigned long addr, unsigned long *sel);
long frame2sel(struct phy_frame *frame, unsigned long *sel);
long pa2frame(unsigned long addr, struct phy_frame **frame);
long frame2pa(struct phy_frame *frame, unsigned long *addr);
long phy_frame_alloc(struct phy_frame **frame);
long phy_frame_ref_desc(struct phy_frame *frame);
long phy_frame_ref_asc(struct phy_frame *frame);

#endif

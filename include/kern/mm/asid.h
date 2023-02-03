#ifndef _KERN_MM_ASID_H_
#define _KERN_MM_ASID_H_

extern unsigned long asid_max;
void asid_init(void);
long asid_alloc(unsigned long *asid);
long asid_free(unsigned long asid);

#endif

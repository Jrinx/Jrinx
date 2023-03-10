#ifndef _KERN_MM_ASID_H_
#define _KERN_MM_ASID_H_

void asid_init(void);
unsigned long asid_get_max(void);
long asid_alloc(unsigned long *asid);
long asid_free(unsigned long asid);

#endif

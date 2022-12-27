#ifndef _KERN_LIB_HART_H_
#define _KERN_LIB_HART_H_

#include <kern/lib/regs.h>

#define HARTID_MAX -1UL

static inline unsigned long hrt_get_id(void) {
  return r_tp();
}

static inline void hrt_set_id(unsigned long hartid) {
  w_tp(hartid);
}

#endif

#ifndef _KERN_LOCK_SPINLOCK_H_
#define _KERN_LOCK_SPINLOCK_H_

#include <kern/lib/errors.h>
#include <kern/lib/hart.h>

enum spinlock_state {
  SPL_FREE = 0,
  SPL_USED = 1,
};

struct spinlock {
  const char *spl_name;
  enum spinlock_state spl_state;
  unsigned long spl_hartid;
};

#define with_spinlock(res)                                                                     \
  struct spinlock res##_splk = {                                                               \
      .spl_name = #res "_splk", .spl_state = SPL_FREE, .spl_hartid = HARTID_MAX}

long spl_acquire(struct spinlock *lock);
long spl_release(struct spinlock *lock);

#endif

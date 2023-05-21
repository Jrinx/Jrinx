#ifndef _KERN_LOCK_LOCK_H_
#define _KERN_LOCK_LOCK_H_

#include <attr.h>

enum lock_state_t {
  LK_UNLOCKED,
  LK_LOCKED,
};

struct lock {
  enum lock_state_t lk_state;
  unsigned long lk_hartid;
  const char *lk_type;
};

typedef long (*lk_aqrl_t)(struct lock *lock);

struct lock_impl_t {
  char *lk_type;
  lk_aqrl_t lk_aq_func;
  lk_aqrl_t lk_rl_func;
};

long lk_acquire(struct lock *lock) __warn_unused_result;
long lk_release(struct lock *lock) __warn_unused_result;

#define lock_init(impl)                                                                        \
  struct lock_impl_t *impl##_impl __section(".ksec.lock_impl." #impl) __used = &impl

#endif

#ifndef _KERN_LOCK_LOCK_H_
#define _KERN_LOCK_LOCK_H_

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

long lk_acquire(struct lock *lock) __attribute__((warn_unused_result));
long lk_release(struct lock *lock) __attribute__((warn_unused_result));
void lk_init(void);

#endif

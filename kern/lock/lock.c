#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lock/lock.h>
#include <lib/string.h>
#include <stddef.h>

static int lk_match_type(const char *type, lk_aqrl_t *aq_func, lk_aqrl_t *rl_func) {
  extern struct lock_impl_t *kern_lock_impl_begin[];
  extern struct lock_impl_t *kern_lock_impl_end[];
  for (struct lock_impl_t **ptr = kern_lock_impl_begin; ptr < kern_lock_impl_end; ptr++) {
    struct lock_impl_t *impl = *ptr;
    if (strcmp(impl->lk_type, type) == 0) {
      *aq_func = impl->lk_aq_func;
      *rl_func = impl->lk_rl_func;
      return 1;
    }
  }
  return 0;
}

long lk_acquire(struct lock *lock) {
  if (lock->lk_state == LK_LOCKED && lock->lk_hartid == hrt_get_id()) {
    return -KER_LOCK_ER;
  }

  lk_aqrl_t aq_func;
  lk_aqrl_t rl_func;

  if (!lk_match_type(lock->lk_type, &aq_func, &rl_func)) {
    return -KER_LOCK_ER;
  }

  catch_e(aq_func(lock));
  lock->lk_hartid = hrt_get_id();

  return KER_SUCCESS;
}

long lk_release(struct lock *lock) {
  if (lock->lk_state == LK_UNLOCKED || lock->lk_hartid != hrt_get_id()) {
    return -KER_LOCK_ER;
  }

  lock->lk_hartid = HARTID_MAX;

  lk_aqrl_t aq_func;
  lk_aqrl_t rl_func;

  if (!lk_match_type(lock->lk_type, &aq_func, &rl_func)) {
    return -KER_LOCK_ER;
  }

  catch_e(rl_func(lock));

  return KER_SUCCESS;
}

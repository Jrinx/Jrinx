#include <kern/lib/hart.h>
#include <kern/lock/spinlock.h>

long spl_acquire(struct spinlock *lock) {
  if (lock->spl_state == SPL_USED && lock->spl_hartid == hrt_get_id()) {
    return -KER_LOCK_ER;
  }

  while (__sync_lock_test_and_set(&lock->spl_state, SPL_USED) != 0) {
  }
  __sync_synchronize();

  lock->spl_hartid = hrt_get_id();

  return KER_SUCCESS;
}

long spl_release(struct spinlock *lock) {
  if (lock->spl_state == SPL_FREE || lock->spl_hartid != hrt_get_id()) {
    return -KER_LOCK_ER;
  }

  lock->spl_hartid = HARTID_MAX;

  __sync_synchronize();
  __sync_lock_release(&lock->spl_state);

  return KER_SUCCESS;
}

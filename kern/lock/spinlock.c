#include <kern/lib/hart.h>
#include <kern/lock/spinlock.h>

long spnlk_acquire(struct lock *lock) {
  while (__sync_lock_test_and_set(&lock->lk_state, LK_LOCKED) != 0) {
  }
  __sync_synchronize();

  return KER_SUCCESS;
}

long spnlk_release(struct lock *lock) {
  __sync_synchronize();
  __sync_lock_release(&lock->lk_state);

  return KER_SUCCESS;
}

#include <kern/lib/hart.h>
#include <kern/lock/spinlock.h>

long spnlk_acquire(struct lock *lock) {
  int wait;
  enum lock_state_t state = LK_LOCKED;
  do {
    asm volatile("amoswap.w %0, %2, %1\n"
                 "fence r, rw"
                 : "=r"(wait), "+A"(lock->lk_state)
                 : "r"(state)
                 : "memory");
  } while (wait);

  return KER_SUCCESS;
}

long spnlk_release(struct lock *lock) {
  enum lock_state_t state = LK_UNLOCKED;
  asm volatile("fence rw, r\n"
               "amoswap.w x0, %1, %0"
               : "+A"(lock->lk_state)
               : "r"(state)
               : "memory");
  return KER_SUCCESS;
}

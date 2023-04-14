#include <brpred.h>
#include <kern/drivers/cpus.h>
#include <kern/lib/errors.h>
#include <kern/lib/hart.h>
#include <kern/lib/regs.h>
#include <kern/lock/lock.h>
#include <kern/lock/spinlock.h>
#include <kern/traps/traps.h>

static long spnlk_acquire(struct lock *lock) {
  int wait;
  enum lock_state_t state = LK_LOCKED;
  if (likely(cpus_intp_layer != NULL)) {
    if (cpus_intp_layer[hrt_get_id()] == 0) {
      rv64_sstatus sstatus = {.val = csrr_sstatus()};
      cpus_retained_intp[hrt_get_id()] = sstatus.bits.sie;
    }
    cpus_intp_layer[hrt_get_id()]++;
    disable_int();
  }
  do {
    asm volatile("amoswap.w %0, %2, %1\n"
                 "fence r, rw"
                 : "=r"(wait), "+A"(lock->lk_state)
                 : "r"(state)
                 : "memory");
  } while (wait);

  return KER_SUCCESS;
}

static long spnlk_release(struct lock *lock) {
  enum lock_state_t state = LK_UNLOCKED;
  asm volatile("fence rw, r\n"
               "amoswap.w x0, %1, %0"
               : "+A"(lock->lk_state)
               : "r"(state)
               : "memory");
  if (likely(cpus_intp_layer != NULL)) {
    cpus_intp_layer[hrt_get_id()]--;
    if (cpus_intp_layer[hrt_get_id()] == 0 && cpus_retained_intp[hrt_get_id()]) {
      enable_int();
    }
  }

  return KER_SUCCESS;
}

static struct lock_impl_t spinlock = {
    .lk_type = SPINLOCK_TYPE,
    .lk_aq_func = spnlk_acquire,
    .lk_rl_func = spnlk_release,
};

lock_init(spinlock);

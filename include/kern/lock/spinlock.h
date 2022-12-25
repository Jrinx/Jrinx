#ifndef _KERN_LOCK_SPINLOCK_H_
#define _KERN_LOCK_SPINLOCK_H_

#include <kern/lib/errors.h>
#include <kern/lib/hart.h>
#include <kern/lock/lock.h>

#define SPINLOCK_TYPE "spin"

#define spinlock_of(res) res##_spnlk

#define with_spinlock(res)                                                                     \
  struct lock spinlock_of(res) = {                                                             \
      .lk_state = LK_UNLOCKED, .lk_hartid = HARTID_MAX, .lk_type = SPINLOCK_TYPE}

#ifdef _KERN_LOCK_SPINLOCK_FUNCDEF_

long spnlk_acquire(struct lock *lock);
long spnlk_release(struct lock *lock);

#endif

#endif

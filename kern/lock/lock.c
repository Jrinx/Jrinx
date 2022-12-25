#include <kern/lib/debug.h>
#include <kern/lock/lock.h>
#define _KERN_LOCK_SPINLOCK_FUNCDEF_
#include <kern/lock/spinlock.h>
#include <lib/string.h>
#include <stddef.h>

#define LOCK_MAP_MAX_SIZE 32U

static size_t lk_map_size = 0;

static const char *lk_type_map[LOCK_MAP_MAX_SIZE];

static lk_aqrl_t lk_aq_map[LOCK_MAP_MAX_SIZE];

static lk_aqrl_t lk_rl_map[LOCK_MAP_MAX_SIZE];

static size_t lk_match_type(const char *type) {
  for (size_t i = 0; i < lk_map_size; i++) {
    if (strcmp(lk_type_map[i], type) == 0) {
      return i;
    }
  }
  return lk_map_size;
}

static long lk_register(const char *type, lk_aqrl_t aq_func, lk_aqrl_t rl_func) {
  assert(lk_map_size < LOCK_MAP_MAX_SIZE);

  size_t i = lk_match_type(type);
  if (i != lk_map_size) {
    return -KER_LOCK_ER;
  }

  lk_type_map[lk_map_size] = type;
  lk_aq_map[lk_map_size] = aq_func;
  lk_rl_map[lk_map_size] = rl_func;

  lk_map_size++;

  return KER_SUCCESS;
}

long lk_acquire(struct lock *lock) {
  if (lock->lk_state == LK_LOCKED && lock->lk_hartid == hrt_get_id()) {
    return -KER_LOCK_ER;
  }

  size_t i = lk_match_type(lock->lk_type);
  assert(i < lk_map_size);

  catch_e(lk_aq_map[i](lock));
  lock->lk_hartid = hrt_get_id();

  return KER_SUCCESS;
}

long lk_release(struct lock *lock) {
  if (lock->lk_state == LK_UNLOCKED || lock->lk_hartid != hrt_get_id()) {
    return -KER_LOCK_ER;
  }

  lock->lk_hartid = HARTID_MAX;

  size_t i = lk_match_type(lock->lk_type);
  assert(i < lk_map_size);

  catch_e(lk_rl_map[i](lock));

  return KER_SUCCESS;
}

void lk_init(void) {
  lk_register(SPINLOCK_TYPE, spnlk_acquire, spnlk_release);
}

#include <kern/lib/errors.h>
#include <stddef.h>

static const char *messages[KER_ERR_MAX] = {
    [KER_SUCCESS] = "success",
    [KER_LOCK_ER] = "lock error",
};

inline const char *msg_of(long err) {
  if (-err >= 0 && -err < KER_ERR_MAX) {
    return messages[-err];
  }
  return NULL;
}

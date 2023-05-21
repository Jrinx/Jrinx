#ifndef _KERN_LIB_DEBUG_H_
#define _KERN_LIB_DEBUG_H_

#include <brpred.h>
#include <builtin.h>
#include <kern/lib/errors.h>
#include <kern/lib/logger.h>
#include <magicros.h>

#define UNIMPLEMENTED fatal("unimplemented: %s", __func__)

#define assert(expr)                                                                           \
  ({                                                                                           \
    if (unlikely(!(expr))) {                                                                   \
      fatal("assertion failed: " #expr "\n");                                                  \
    }                                                                                          \
  })

#define _catch_e_with_action(expr, action)                                                     \
  ({                                                                                           \
    typeof(expr) ret = (expr);                                                                 \
    long err = *((long *)&ret);                                                                \
    if (unlikely(err < 0)) {                                                                   \
      (action);                                                                                \
      __unreachable();                                                                         \
    }                                                                                          \
  })

#define catch_e(...) MACRO_CONCAT(_catch_e_, VA_CNT(__VA_ARGS__))(__VA_ARGS__)
#define _catch_e_1(x1) _catch_e_with_action(x1, { return err; })
#define _catch_e_2(x1, x2) _catch_e_with_action(x1, x2)

#define panic_e(expr)                                                                          \
  catch_e(expr, {                                                                              \
    if (sizeof(ret) == sizeof(long) && -err < KER_ERR_MAX) {                                   \
      fatal("unexpected error %ld (%s): " #expr, err, msg_of(err));                            \
    } else {                                                                                   \
      fatal("unexpected error %ld: " #expr, err);                                              \
    }                                                                                          \
  })

#endif

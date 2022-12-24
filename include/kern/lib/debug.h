#ifndef _KERN_LIB_DEBUG_H_
#define _KERN_LIB_DEBUG_H_

#include <brpred.h>
#include <kern/lib/logger.h>

#define assert(expr)                                                                           \
  ({                                                                                           \
    if (unlikely(expr)) {                                                                      \
      fatal("assertion failed: " #expr);                                                       \
    }                                                                                          \
  })

#define catch_e(expr)                                                                          \
  ({                                                                                           \
    typeof(expr) ret = (expr);                                                                 \
    long err = *((long *)&ret);                                                                \
    if (unlikely(err < 0)) {                                                                   \
      return err;                                                                              \
    }                                                                                          \
  })

#define panic_e(expr)                                                                          \
  ({                                                                                           \
    typeof(expr) ret = (expr);                                                                 \
    long err = *((long *)&ret);                                                                \
    if (unlikely(err < 0)) {                                                                   \
      fatal("unexpected error %ld: " #expr, err);                                              \
    }                                                                                          \
  })

#endif

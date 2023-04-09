#ifndef _USER_LIB_DEBUG_H_
#define _USER_LIB_DEBUG_H_

#include <types.h>
#include <user/lib/logger.h>

#define check_e(expr)                                                                          \
  ({                                                                                           \
    RETURN_CODE_TYPE ret;                                                                      \
    (expr);                                                                                    \
    if (ret != NO_ERROR) {                                                                     \
      fatal("expected error: %d\n", ret);                                                      \
    }                                                                                          \
  })

#endif

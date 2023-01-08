#ifndef _CALLBACK_H_
#define _CALLBACK_H_

#include <magicros.h>

#define cb_typedef(func_t)                                                                     \
  struct {                                                                                     \
    func_t cb_func;                                                                            \
    void *cb_ctx;                                                                              \
  }

#define cb_decl(cb_type, cb_name, func, ctx)                                                   \
  cb_type cb_name = {                                                                          \
      .cb_func = func,                                                                         \
      .cb_ctx = ctx,                                                                           \
  }

#define cb_invoke(cb_st) CURRYING((cb_st).cb_func, (cb_st).cb_ctx)

#endif

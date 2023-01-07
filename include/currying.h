#ifndef _CURRYING_H_
#define _CURRYING_H_

#define _currying_1n_head(head) ((head)
#define _currying_1n_tail(...) , ##__VA_ARGS__)
#define currying_1n(func, head) (func) _currying_1n_head(head) _currying_1n_tail

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

#define cb_invoke(cb_st) currying_1n((cb_st).cb_func, (cb_st).cb_ctx)

#endif

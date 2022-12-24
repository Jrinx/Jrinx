#ifndef _KERN_LIB_ERRORS_H_
#define _KERN_LIB_ERRORS_H_

enum {
  KER_SUCCESS = 0,
  KER_LOCK_ER,
  KER_ERR_MAX,
};

const char *msg_of(long err);

#endif

#ifndef _KERN_LIB_ERRORS_H_
#define _KERN_LIB_ERRORS_H_

enum {
  KER_SUCCESS = 0,
  KER_ARG_ER,
  KER_LOCK_ER,
  KER_DTB_ER,
  KER_ELF_ER,
  KER_DEV_ER,
  KER_MEM_ER,
  KER_CTX_ER,
  KER_INT_ER,
  KER_SRL_ER,
  KER_ASID_ER,
  KER_PART_ER,
  KER_PROC_ER,
  KER_SCHED_ER,
  KER_PORT_ER,
  KER_CHAN_ER,
  KER_BUF_ER,
  KER_ERR_MAX,
};

const char *msg_of(long err);

#endif

#ifndef _KERN_CHAN_CHANNEL_H_
#define _KERN_CHAN_CHANNEL_H_

#include <kern/lock/lock.h>
#include <list.h>
#include <types.h>

enum channel_media {
  CM_MEMORY,
};

enum channel_type {
  CT_SAMPLING,
  CT_QUEUING,
};

struct channel {
  enum channel_media ch_media;
  enum channel_type ch_type;
  void *ch_data;
  size_t ch_cap;
  union {
    struct {
      msg_size_t ch_max_msg_size;
      msg_range_t ch_max_nb_msg;
      long ch_off_b;
      long ch_off_e;
      msg_range_t ch_nb_msg;
      struct list_head ch_waiting_procs;
    } queuing;
  } ch_view;
  struct lock ch_lock;
};

struct channel_conf {
  enum channel_media cc_media;
  enum channel_type cc_type;
  char **cc_port_names;
};

long channel_create(struct channel_conf *cc) __attribute__((warn_unused_result));
long channel_mem_setup(void) __attribute__((warn_unused_result));

#endif

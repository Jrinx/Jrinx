#ifndef _KERNEL_COMM_MSG_H_
#define _KERNEL_COMM_MSG_H_

#include <types.h>

struct comm_msg {
  msg_size_t msg_size;
  uint8_t msg_data[];
};

#endif

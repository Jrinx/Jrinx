#ifndef _KERN_TRAPS_TIMER_H_
#define _KERN_TRAPS_TIMER_H_

#include <kern/comm/buffer.h>
#include <kern/multitask/partition.h>
#include <kern/multitask/process.h>
#include <list.h>
#include <types.h>

enum time_event_type {
  TE_PROCESS_SUSPEND_TIMEOUT = 0b1,
  TE_PROCESS_DELAYED_START = 0b10,
  TE_BUFFER_BLOCK_TIMEOUT = 0b100,
  TE_ANY = ~0U,
};

struct time_event {
  sys_time_t te_time;
  void *te_ctx;
  enum time_event_type te_type;
  struct linked_node te_queue_link;
};

struct te_proc_buf {
  struct proc *tepb_proc;
  struct buffer *tepb_buf;
};

void time_event_init(void);
void time_event_alloc(void *ctx, sys_time_t time, enum time_event_type type);
void time_event_free(struct time_event *te);
void time_event_action(void);

#endif

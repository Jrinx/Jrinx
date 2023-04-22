#ifndef _KERN_TRAPS_TIMER_H_
#define _KERN_TRAPS_TIMER_H_

#include <kern/chan/queuing.h>
#include <kern/comm/buffer.h>
#include <kern/multitask/partition.h>
#include <kern/multitask/process.h>
#include <list.h>
#include <types.h>

enum time_event_type {
  TE_PARTITION_ACTIVATE,
  TE_PROCESS_SUSPEND_TIMEOUT,
  TE_PROCESS_DELAYED_START,
  TE_BUFFER_BLOCK_TIMEOUT,
  TE_QUEUING_PORT_BLOCK_TIMEOUT,
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

struct te_proc_queuing_port {
  struct proc *tepqp_proc;
  struct queuing_port *tepqp_port;
};

void time_event_init(void);
void time_event_alloc(void *ctx, sys_time_t time, enum time_event_type type);
void time_event_free(struct time_event *te);
void time_event_action(void);

#endif

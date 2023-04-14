#include <kern/comm/buffer.h>
#include <kern/drivers/cpus.h>
#include <kern/lib/boottime.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lib/sbi.h>
#include <kern/lock/lock.h>
#include <kern/lock/spinlock.h>
#include <kern/mm/kalloc.h>
#include <kern/multitask/process.h>
#include <kern/multitask/sched.h>
#include <kern/traps/timer.h>
#include <lib/string.h>
#include <list.h>
#include <types.h>

static struct list_head *cpus_time_event_queue;

void time_event_init(void) {
  cpus_time_event_queue = kalloc(sizeof(struct list_head) * cpus_get_count());
  for (size_t i = 0; i < cpus_get_count(); i++) {
    list_init(&cpus_time_event_queue[i]);
  }
}

static void time_event_set_head(void) {
  struct time_event *te;
  if (list_empty(&cpus_time_event_queue[hrt_get_id()])) {
    panic_e(sbi_set_timer(UINT64_MAX - 1));
    return;
  }
  te = CONTAINER_OF(cpus_time_event_queue[hrt_get_id()].l_first, struct time_event,
                    te_queue_link);
  panic_e(sbi_set_timer(te->te_time * (cpus_get_timebase_freq() / SYS_TIME_SECOND)));
}

void time_event_alloc(void *ctx, sys_time_t time, enum time_event_type type) {
  struct time_event *te = kalloc(sizeof(struct time_event));
  memset(te, 0, sizeof(struct time_event));
  te->te_ctx = ctx;
  te->te_time = time;
  te->te_type = type;
  struct time_event *i;
  LINKED_NODE_ITER (cpus_time_event_queue[hrt_get_id()].l_first, i, te_queue_link) {
    if (i->te_time > te->te_time) {
      list_insert_before(&i->te_queue_link, &te->te_queue_link);
      goto succ;
    }
  }
  list_insert_tail(&cpus_time_event_queue[hrt_get_id()], &te->te_queue_link);
succ:
  time_event_set_head();
  switch (type) {
  case TE_PROCESS_SUSPEND_TIMEOUT:
  case TE_PROCESS_DELAYED_START:
    struct proc *proc = ctx;
    proc->pr_asso_timer = te;
    break;
  case BUFFER_BLOCKED:
    struct te_proc_buf *tepb = ctx;
    tepb->tepb_proc->pr_asso_timer = te;
    break;
  default:
    break;
  }
}

void time_event_free(struct time_event *te) {
  struct time_event *i;
  LINKED_NODE_ITER (cpus_time_event_queue[hrt_get_id()].l_first, i, te_queue_link) {
    if (i == te) {
      list_remove_node(&cpus_time_event_queue[hrt_get_id()], &te->te_queue_link);
      break;
    }
  }
  time_event_set_head();
  switch (te->te_type) {
  case TE_PROCESS_SUSPEND_TIMEOUT:
  case TE_PROCESS_DELAYED_START:
    struct proc *proc = te->te_ctx;
    proc->pr_asso_timer = NULL;
    break;
  case BUFFER_BLOCKED:
    struct te_proc_buf *tepb = te->te_ctx;
    tepb->tepb_proc->pr_asso_timer = NULL;
    break;
  default:
    break;
  }
  kfree(te);
}

void time_event_action(void) {
  while (!list_empty(&cpus_time_event_queue[hrt_get_id()])) {
    struct time_event *te = CONTAINER_OF(cpus_time_event_queue[hrt_get_id()].l_first,
                                         struct time_event, te_queue_link);
    if (te->te_time <= boottime_get_now()) {
      switch (te->te_type) {
      case TE_PROCESS_SUSPEND_TIMEOUT:
      case TE_PROCESS_DELAYED_START:
        struct proc *proc = te->te_ctx;
        proc->pr_state = READY;
        break;
      case BUFFER_BLOCKED:
        struct te_proc_buf *tepb = te->te_ctx;
        tepb->tepb_proc->pr_state = READY;
        buffer_del_waiting_proc(tepb->tepb_buf, tepb->tepb_proc);
        break;
      default:
        break;
      }
      time_event_free(te);
    } else {
      break;
    }
  }
  sched_proc();
}

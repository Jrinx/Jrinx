#include <aligns.h>
#include <kern/chan/queuing.h>
#include <kern/comm/buffer.h>
#include <kern/drivers/cpus.h>
#include <kern/lib/boottime.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lib/sbi.h>
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
  assert(boottime_get_now() < te->te_time);
  panic_e(sbi_set_timer(te->te_time * (cpus_get_timebase_freq() / SYS_TIME_SECOND)));
}

void time_event_alloc(void *ctx, sys_time_t time, enum time_event_type type) {
  struct time_event *te = kalloc(sizeof(struct time_event));
  memset(te, 0, sizeof(struct time_event));
  te->te_ctx = ctx;
  te->te_time = align_up(time, SYS_TIME_MIN_TICK);
  te->te_type = type;
  struct time_event *i;
  intp_push();
  LINKED_NODE_ITER (cpus_time_event_queue[hrt_get_id()].l_first, i, te_queue_link) {
    if (i->te_time > te->te_time) {
      list_insert_before(&i->te_queue_link, &te->te_queue_link);
      goto succ;
    }
  }
  list_insert_tail(&cpus_time_event_queue[hrt_get_id()], &te->te_queue_link);
succ:
  intp_pop();
  time_event_set_head();
  switch (type) {
  case TE_PROCESS_SUSPEND_TIMEOUT:
  case TE_PROCESS_DELAYED_START:
  case TE_PROCESS_TIMED_WAIT_TIMEOUT:
    struct proc *proc = ctx;
    proc->pr_asso_timer = te;
    break;
  case TE_BUFFER_BLOCK_TIMEOUT:
    struct te_proc_buf *tepb = ctx;
    tepb->tepb_proc->pr_asso_timer = te;
    break;
  case TE_BLACKBOARD_BLOCK_TIMEOUT:
    struct te_proc_bb *tepbb = ctx;
    tepbb->tepbb_proc->pr_asso_timer = te;
    break;
  case TE_QUEUING_PORT_BLOCK_TIMEOUT:
    struct te_proc_queuing_port *tepqp = ctx;
    tepqp->tepqp_proc->pr_asso_timer = te;
    break;
  default:
    break;
  }
}

void time_event_free(struct time_event *te) {
  struct time_event *i;
  intp_push();
  LINKED_NODE_ITER (cpus_time_event_queue[hrt_get_id()].l_first, i, te_queue_link) {
    if (i == te) {
      list_remove_node(&cpus_time_event_queue[hrt_get_id()], &te->te_queue_link);
      break;
    }
  }
  intp_pop();
  time_event_set_head();
  switch (te->te_type) {
  case TE_PROCESS_SUSPEND_TIMEOUT:
  case TE_PROCESS_DELAYED_START:
  case TE_PROCESS_TIMED_WAIT_TIMEOUT:
    struct proc *proc = te->te_ctx;
    proc->pr_asso_timer = NULL;
    break;
  case TE_BUFFER_BLOCK_TIMEOUT:
    struct te_proc_buf *tepb = te->te_ctx;
    tepb->tepb_proc->pr_asso_timer = NULL;
    break;
  case TE_BLACKBOARD_BLOCK_TIMEOUT:
    struct te_proc_bb *tepbb = te->te_ctx;
    tepbb->tepbb_proc->pr_asso_timer = NULL;
    break;
  case TE_QUEUING_PORT_BLOCK_TIMEOUT:
    struct te_proc_queuing_port *tepqp = te->te_ctx;
    tepqp->tepqp_proc->pr_asso_timer = NULL;
    break;
  default:
    break;
  }
  kfree(te);
}

void time_event_action(void) {
  int resched_part = 0;
  int resched_proc = 0;
  while (!list_empty(&cpus_time_event_queue[hrt_get_id()])) {
    struct time_event *te = CONTAINER_OF(cpus_time_event_queue[hrt_get_id()].l_first,
                                         struct time_event, te_queue_link);
    if (te->te_time <= boottime_get_now() + CONFIG_FIXTICK) {
      switch (te->te_type) {
      case TE_PARTITION_ACTIVATE:
        resched_part = 1;
        break;
      case TE_PROCESS_SUSPEND_TIMEOUT:
      case TE_PROCESS_DELAYED_START:
      case TE_PROCESS_TIMED_WAIT_TIMEOUT:
        struct proc *proc = te->te_ctx;
        proc->pr_state = READY;
        proc->pr_asso_timer = NULL;
        if (proc->pr_part_id == sched_cur_part()->pa_id) {
          resched_proc = 1;
        }
        break;
      case TE_BUFFER_BLOCK_TIMEOUT:
        struct te_proc_buf *tepb = te->te_ctx;
        tepb->tepb_proc->pr_state = READY;
        tepb->tepb_proc->pr_asso_timer = NULL;
        buffer_del_waiting_proc(tepb->tepb_buf, tepb->tepb_proc);
        if (tepb->tepb_proc->pr_part_id == sched_cur_part()->pa_id) {
          resched_proc = 1;
        }
        break;
      case TE_BLACKBOARD_BLOCK_TIMEOUT:
        struct te_proc_bb *tepbb = te->te_ctx;
        tepbb->tepbb_proc->pr_state = READY;
        tepbb->tepbb_proc->pr_asso_timer = NULL;
        blackboard_del_waiting_proc(tepbb->tepbb_bb, tepbb->tepbb_proc);
        if (tepbb->tepbb_proc->pr_part_id == sched_cur_part()->pa_id) {
          resched_proc = 1;
        }
        break;
      case TE_QUEUING_PORT_BLOCK_TIMEOUT:
        struct te_proc_queuing_port *tepqp = te->te_ctx;
        tepqp->tepqp_proc->pr_state = READY;
        tepqp->tepqp_proc->pr_asso_timer = NULL;
        queuing_port_del_waiting_proc(tepqp->tepqp_port, tepqp->tepqp_proc);
        if (tepqp->tepqp_proc->pr_part_id == sched_cur_part()->pa_id) {
          resched_proc = 1;
        }
        break;
      default:
        break;
      }
      list_remove_node(&cpus_time_event_queue[hrt_get_id()], &te->te_queue_link);
      kfree(te);
    } else {
      break;
    }
  }
  time_event_set_head();
  if (resched_part) {
    sched_global();
  }
  if (resched_proc) {
    sched_proc(0);
  }
}

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
static struct lock *cpus_time_event_queue_lock;

void time_event_init(void) {
  cpus_time_event_queue = kalloc(sizeof(struct list_head) * cpus_get_count());
  cpus_time_event_queue_lock = kalloc(sizeof(struct lock) * cpus_get_count());
  for (size_t i = 0; i < cpus_get_count(); i++) {
    list_init(&cpus_time_event_queue[i]);
    spinlock_init(&cpus_time_event_queue_lock[i]);
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
  panic_e(lk_acquire(&cpus_time_event_queue_lock[hrt_get_id()]));
  LINKED_NODE_ITER (cpus_time_event_queue[hrt_get_id()].l_first, i, te_queue_link) {
    if (i->te_time > te->te_time) {
      list_insert_before(&i->te_queue_link, &te->te_queue_link);
      goto succ;
    }
  }
  list_insert_tail(&cpus_time_event_queue[hrt_get_id()], &te->te_queue_link);
succ:
  time_event_set_head();
  panic_e(lk_release(&cpus_time_event_queue_lock[hrt_get_id()]));
  switch (type) {
  case TE_PROCESS_SUSPEND_TIMEOUT:
  case TE_PROCESS_DELAYED_START:
    struct proc *proc = ctx;
    hlist_insert_head(&proc->pr_time_events, &te->te_proc_link);
    break;
  default:
    break;
  }
}

static void time_event_free(struct time_event *te) {
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
    hlist_remove_node(&te->te_proc_link);
    break;
  default:
    break;
  }
  kfree(te);
}

int time_event_proc_has_type(struct proc *proc, enum time_event_type type) {
  struct time_event *te;
  LINKED_NODE_ITER (proc->pr_time_events.h_first, te, te_proc_link) {
    if (te->te_type == type) {
      return 1;
    }
  }
  return 0;
}

void time_event_proc_free_filter_type(struct proc *proc, unsigned type) {
  while (!hlist_empty(&proc->pr_time_events)) {
    struct time_event *te;
    struct time_event *rm = NULL;
    LINKED_NODE_ITER (proc->pr_time_events.h_first, te, te_proc_link) {
      if (te->te_type & type) {
        rm = te;
        break;
      }
    }
    if (rm == NULL) {
      break;
    }
    time_event_free(te);
  }
}

void time_event_action(void) {
  panic_e(lk_acquire(&cpus_time_event_queue_lock[hrt_get_id()]));
  while (!list_empty(&cpus_time_event_queue[hrt_get_id()])) {
    struct time_event *te = CONTAINER_OF(cpus_time_event_queue[hrt_get_id()].l_first,
                                         struct time_event, te_queue_link);
    if (te->te_time <= boottime_get_now()) {
      switch (te->te_type) {
      case TE_PROCESS_SUSPEND_TIMEOUT:
      case TE_PROCESS_DELAYED_START:
        struct proc *proc = te->te_ctx;
        proc->pr_state = READY;
      default:
        break;
      }
      time_event_free(te);
    } else {
      break;
    }
  }
  panic_e(lk_release(&cpus_time_event_queue_lock[hrt_get_id()]));
  sched_proc_give_up();
}

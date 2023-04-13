#ifndef _KERN_MULTITASK_PROCESS_H_
#define _KERN_MULTITASK_PROCESS_H_

#include <kern/lib/regs.h>
#include <kern/multitask/partition.h>
#include <kern/traps/timer.h>
#include <kern/traps/traps.h>
#include <list.h>
#include <stdint.h>
#include <types.h>

typedef enum {
  PREVIOUS_STARTED,
  DELAYED_STARTED,
  SUSPENDED,
  SUSPENDED_WITH_TIMEOUT,
} proc_waiting_reason_t;

struct proc {
  struct trapframe pr_trapframe;
  char *pr_name;
  proc_id_t pr_id;
  proc_index_t pr_idx;
  part_id_t pr_part_id;
  sys_addr_t pr_ustacktop;
  stack_size_t pr_ustacksize;
  sys_addr_t pr_entrypoint;
  priority_t pr_base_pri;
  sys_time_t pr_period;
  sys_time_t pr_time_cap;
  deadline_t pr_deadline;
  priority_t pr_cur_pri;
  sys_time_t pr_deadline_time;
  proc_state_t pr_state;
  proc_waiting_reason_t pr_waiting_reason;
  proc_core_id_t pr_core_id;
  struct time_event *pr_asso_timer;
  struct linked_node pr_id_link;
  struct linked_node pr_sched_link;
  struct linked_node pr_name_link;
};

struct proc *proc_from_id(proc_id_t pr_id);
void proc_reset(struct proc *proc);
long proc_alloc(struct part *part, struct proc **proc, const char *name, sys_time_t period,
                sys_time_t time_cap, sys_addr_t entrypoint, stack_size_t stacksize,
                priority_t base_pri, deadline_t deadline);
long proc_free(struct proc *proc);
void proc_run(struct proc *proc) __attribute__((noreturn));

static inline int proc_is_period(struct proc *proc) {
  return proc->pr_period != SYSTEM_TIME_INFINITE_VAL;
}

#endif

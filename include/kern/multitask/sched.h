#ifndef _KERN_MULTITASK_SCHED_H_
#define _KERN_MULTITASK_SCHED_H_

#include <kern/lib/debug.h>
#include <kern/multitask/process.h>

extern struct part **cpus_cur_part;
extern struct proc **cpus_cur_proc;

void sched_init(void);
void sched_add_part(struct part *part);
void sched_global(void) __attribute__((noreturn));
void sched_proc(void) __attribute__((noreturn));
void sched_proc_give_up();

static inline struct proc *sched_cur_proc(void) {
  return cpus_cur_proc[hrt_get_id()];
}

static inline struct part *sched_cur_part(void) {
  return cpus_cur_part[hrt_get_id()];
}

#endif

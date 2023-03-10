#ifndef _KERN_MULTITASK_SCHED_H_
#define _KERN_MULTITASK_SCHED_H_

#include <kern/multitask/process.h>

extern struct proc **cpus_cur_proc;

void sched_init(void);
long sched_assign_proc(unsigned long hartid, struct proc *proc)
    __attribute__((warn_unused_result));
int sched_has_proc(void);
void sched(void) __attribute__((noreturn));

#endif

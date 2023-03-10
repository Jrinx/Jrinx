#ifndef _KERN_MULTITASK_PROCESS_H_
#define _KERN_MULTITASK_PROCESS_H_

#include <kern/lib/regs.h>
#include <kern/multitask/partition.h>
#include <kern/traps/traps.h>
#include <list.h>
#include <stdint.h>

struct proc {
  struct trapframe pr_trapframe;
  uint64_t pr_id;
  uint64_t pr_part_id;
  const char *pr_name;
  unsigned long pr_ustacktop;
  size_t pr_ustacksize;
  unsigned long pr_entrypoint;
  struct linked_node pr_id_link;
  struct linked_node pr_sched_link;
};

long proc_alloc(struct part *part, struct proc **proc, const char *name,
                unsigned long entrypoint, size_t stacksize);
long proc_free(struct proc *proc);
void proc_run(struct proc *proc) __attribute__((noreturn));

#endif

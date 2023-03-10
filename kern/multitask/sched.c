#include <kern/drivers/cpus.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lock/lock.h>
#include <kern/lock/spinlock.h>
#include <kern/mm/pmm.h>
#include <kern/multitask/process.h>
#include <lib/string.h>

struct proc **cpus_cur_proc;
static struct list_head *cpus_sched_list;
static with_spinlock(cpus_sched_list);

void sched_init(void) {
  cpus_cur_proc = alloc(sizeof(struct proc *) * cpus_get_count(), sizeof(struct proc *));
  cpus_sched_list =
      alloc(sizeof(struct list_head) * cpus_get_count(), sizeof(struct list_head));
  memset(cpus_cur_proc, 0, sizeof(struct proc *) * cpus_get_count());
  memset(cpus_sched_list, 0, sizeof(struct list_head) * cpus_get_count());
  for (size_t i = 0; i < cpus_get_count(); i++) {
    list_init(&cpus_sched_list[i]);
  }
}

long sched_assign_proc(unsigned long hartid, struct proc *proc) {
  if (hartid >= cpus_get_count()) {
    return -KER_SCHED_ER;
  }
  info("assign '%s' of partition %lu to cpu@%lu\n", proc->pr_name, proc->pr_part_id, hartid);
  panic_e(lk_acquire(&spinlock_of(cpus_sched_list)));
  list_insert_tail(&cpus_sched_list[hartid], &proc->pr_sched_link);
  panic_e(lk_release(&spinlock_of(cpus_sched_list)));
  return KER_SUCCESS;
}

int sched_has_proc(void) {
  return !list_empty(&cpus_sched_list[hrt_get_id()]);
}

void sched(void) {
  extern int args_debug_sched_max_cnt;
  if (args_debug_sched_max_cnt) {
    static int sched_cnt = 0;
    if (sched_cnt >= args_debug_sched_max_cnt) {
      halt("sched count exceeded!\n");
    }
    sched_cnt++;
  }

  struct list_head *sched_list = &cpus_sched_list[hrt_get_id()];
  struct proc **cur_proc = &cpus_cur_proc[hrt_get_id()];
  if (*cur_proc != NULL) {
    list_insert_tail(sched_list, &(*cur_proc)->pr_sched_link);
  }
  *cur_proc = NULL;
  while (list_empty(sched_list)) {
  }
  struct proc *sched_proc = CONTAINER_OF(sched_list->l_first, struct proc, pr_sched_link);
  list_remove_node(sched_list, &sched_proc->pr_sched_link);
  *cur_proc = sched_proc;
  proc_run(sched_proc);
}

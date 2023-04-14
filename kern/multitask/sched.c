#include <kern/drivers/cpus.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lock/lock.h>
#include <kern/lock/spinlock.h>
#include <kern/mm/kalloc.h>
#include <kern/multitask/process.h>
#include <lib/string.h>

struct part **cpus_cur_part;
struct proc **cpus_cur_proc;
static struct list_head sched_part_list;
static with_spinlock(sched_part_list);

void sched_init(void) {
  cpus_cur_part = kalloc(sizeof(struct part *) * cpus_get_count());
  cpus_cur_proc = kalloc(sizeof(struct proc *) * cpus_get_count());
  memset(cpus_cur_part, 0, sizeof(struct part *) * cpus_get_count());
  memset(cpus_cur_proc, 0, sizeof(struct proc *) * cpus_get_count());
  list_init(&sched_part_list);
}

void sched_add_part(struct part *part) {
  info("add partition %lu to scheduler\n", part->pa_id);
  panic_e(lk_acquire(&spinlock_of(sched_part_list)));
  list_insert_tail(&sched_part_list, &part->pa_sched_link);
  panic_e(lk_release(&spinlock_of(sched_part_list)));
}

static struct proc *sched_elect_next_proc(void) {
  struct proc *next_proc = NULL;
  struct proc *proc;
  do {
    LINKED_NODE_ITER (cpus_cur_part[hrt_get_id()]->pa_proc_list.l_first, proc, pr_sched_link) {
      if ((proc->pr_state == READY || proc->pr_state == RUNNING) &&
          (next_proc == NULL || proc->pr_cur_pri > next_proc->pr_cur_pri)) {
        next_proc = proc;
      }
    }
  } while (next_proc == NULL);
  return next_proc;
}

// TODO: impl standard arinc 653 scheduler
__attribute__((noreturn)) void sched_proc(void) {
  struct proc *cur_proc = cpus_cur_proc[hrt_get_id()];
  if (cur_proc != NULL && cur_proc->pr_state == RUNNING) {
    cur_proc->pr_state = READY;
  }
  struct proc *next_proc = sched_elect_next_proc();
  cpus_cur_proc[hrt_get_id()] = next_proc;
  next_proc->pr_state = RUNNING;
  proc_run(next_proc);
}

void sched_global(void) {
  if (cpus_cur_part[hrt_get_id()] != NULL) {
    list_remove_node(&sched_part_list, &cpus_cur_part[hrt_get_id()]->pa_sched_link);
    list_insert_tail(&sched_part_list, &cpus_cur_part[hrt_get_id()]->pa_sched_link);
  }
  assert(!list_empty(&sched_part_list));
  struct part *next_part = CONTAINER_OF(sched_part_list.l_first, struct part, pa_sched_link);
  cpus_cur_part[hrt_get_id()] = next_part;
  cpus_cur_proc[hrt_get_id()] = NULL;
  sched_proc();
}

void sched_proc_give_up() {
  extern int args_debug_sched_max_cnt;
  if (args_debug_sched_max_cnt) {
    static int sched_cnt = 0;
    if (sched_cnt >= args_debug_sched_max_cnt) {
      halt("sched count exceeded!\n");
    }
    sched_cnt++;
  }
  struct proc *proc = cpus_cur_proc[hrt_get_id()];
  struct proc *next_proc = sched_elect_next_proc();
  if (proc != next_proc) {
    struct context *context = kalloc(sizeof(struct context));
    memset(context, 0, sizeof(struct context));
    hlist_insert_head(&proc->pr_trapframe.tf_ctx_list, &context->ctx_link);
    extern void _sched_proc_give_up(struct context * context);
    _sched_proc_give_up(context);
  }
}

#include <arithmetic.h>
#include <kern/drivers/cpus.h>
#include <kern/lib/boottime.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lock/lock.h>
#include <kern/lock/spinlock.h>
#include <kern/mm/kalloc.h>
#include <kern/multitask/partition.h>
#include <kern/multitask/process.h>
#include <kern/multitask/sched.h>
#include <lib/string.h>

struct part **cpus_cur_part;
struct proc **cpus_cur_proc;

static struct list_head sched_modules;
static struct list_head sched_parts;

static struct sched_module *sched_cur_mod;
static sys_time_t sched_major_frame;

void sched_init(void) {
  cpus_cur_part = kalloc(sizeof(struct part *) * cpus_get_count());
  cpus_cur_proc = kalloc(sizeof(struct proc *) * cpus_get_count());
  memset(cpus_cur_part, 0, sizeof(struct part *) * cpus_get_count());
  memset(cpus_cur_proc, 0, sizeof(struct proc *) * cpus_get_count());
  list_init(&sched_modules);
  list_init(&sched_parts);
}

long sched_module_add(struct sched_conf *conf) {
  info("sched module add: part='%s',offset=%lu us\n", conf->sc_pa_name, conf->sc_offset);
  struct part *part = part_from_name(conf->sc_pa_name);
  if (part == NULL) {
    return -KER_PART_ER;
  }
  struct sched_module *module = kalloc(sizeof(struct sched_module));
  module->sm_part = part;
  module->sm_offset = conf->sc_offset;
  module->sm_duration = conf->sc_duration;
  if (!list_empty(&sched_modules)) {
    struct sched_module *last_mod =
        CONTAINER_OF(list_last(&sched_modules), struct sched_module, sm_link);
    if (last_mod->sm_offset + last_mod->sm_duration > module->sm_offset) {
      return -KER_SCHED_ER;
    }
    if (last_mod->sm_offset + last_mod->sm_duration < module->sm_offset) {
      info("nop detected between sched modules '%s'(offset=%lu us) and '%s'(offset=%lu us)\n",
           last_mod->sm_part->pa_name, last_mod->sm_offset, module->sm_part->pa_name,
           module->sm_offset);
    }
  }
  list_insert_tail(&sched_modules, &module->sm_link);
  return KER_SUCCESS;
}

void sched_add_part(struct part *part) {
  list_insert_tail(&sched_parts, &part->pa_sched_link);
}

long sched_launch(void) {
  if (list_empty(&sched_parts)) {
    return -KER_SCHED_ER;
  }
  sched_major_frame = 1;
  struct part *part;
  LINKED_NODE_ITER (sched_parts.l_first, part, pa_sched_link) {
    sched_major_frame = lcm(sched_major_frame, part->pa_period);
  }
  info("sched major frame: %lu us\n", sched_major_frame);
  struct sched_module *mod;
  LINKED_NODE_ITER (sched_modules.l_first, mod, sm_link) {
    if (mod->sm_offset + mod->sm_duration > sched_major_frame) {
      return -KER_SCHED_ER;
    }
  }

  sys_time_t now = boottime_get_now();

  // set timer except for the first module
  LINKED_NODE_ITER (sched_modules.l_first, mod, sm_link) {
    if (&mod->sm_link != sched_modules.l_first) {
      time_event_alloc(NULL, now + mod->sm_offset, TE_PARTITION_ACTIVATE);
    }
    struct sched_module *next_mod =
        CONTAINER_OF(LINKED_NODE_NEXT(mod, sm_link), struct sched_module, sm_link);
    mod->sm_prev_act_time = now + mod->sm_offset;
    if (mod->sm_offset + mod->sm_duration < next_mod->sm_offset) {
      // TODO: set timer for nop?
    }
  }
  return KER_SUCCESS;
}

static struct proc *sched_elect_next_proc(int round_robin) {
  struct proc *next_proc = NULL;
  struct proc *proc;
  do {
    if (round_robin) {
      const priority_t cur_pri = cpus_cur_proc[hrt_get_id()]->pr_cur_pri;
      LINKED_NODE_ITER (cpus_cur_proc[hrt_get_id()]->pr_sched_link.next, proc, pr_sched_link) {
        if ((proc->pr_state == READY || proc->pr_state == RUNNING) &&
            proc->pr_cur_pri >= cur_pri) {
          next_proc = proc;
          goto rr_found;
        }
      }
      LINKED_NODE_ITER (cpus_cur_part[hrt_get_id()]->pa_proc_list.l_first, proc,
                        pr_sched_link) {
        if ((proc->pr_state == READY || proc->pr_state == RUNNING) &&
            proc->pr_cur_pri >= cur_pri) {
          next_proc = proc;
          goto rr_found;
        }
      }
    rr_found:
    } else {
      LINKED_NODE_ITER (cpus_cur_part[hrt_get_id()]->pa_proc_list.l_first, proc,
                        pr_sched_link) {
        if ((proc->pr_state == READY || proc->pr_state == RUNNING) &&
            (next_proc == NULL || proc->pr_cur_pri > next_proc->pr_cur_pri)) {
          next_proc = proc;
        }
      }
    }
  } while (next_proc == NULL);
  return next_proc;
}

// TODO: impl standard arinc 653 scheduler
__noreturn void sched_proc(int round_robin) {
  struct proc *cur_proc = cpus_cur_proc[hrt_get_id()];
  if (cur_proc != NULL && cur_proc->pr_state == RUNNING) {
    cur_proc->pr_state = READY;
  }
  intp_enable();
  struct proc *next_proc = sched_elect_next_proc(round_robin);
  intp_disable();
  cpus_cur_proc[hrt_get_id()] = next_proc;
  next_proc->pr_state = RUNNING;
  proc_run(next_proc);
}

void sched_global(void) {
  struct part *next_part;
  if (unlikely(list_empty(&sched_modules))) {
    assert(!list_empty(&sched_parts));
    next_part = CONTAINER_OF(sched_parts.l_first, struct part, pa_sched_link);
  } else {
    if (unlikely(sched_cur_mod == NULL) || sched_cur_mod->sm_link.next == NULL) {
      sched_cur_mod = CONTAINER_OF(sched_modules.l_first, struct sched_module, sm_link);
    } else {
      sched_cur_mod =
          CONTAINER_OF(LINKED_NODE_NEXT(sched_cur_mod, sm_link), struct sched_module, sm_link);
    }
    next_part = sched_cur_mod->sm_part;
    time_event_alloc(NULL, sched_cur_mod->sm_prev_act_time + sched_major_frame,
                     TE_PARTITION_ACTIVATE);
    sched_cur_mod->sm_prev_act_time += sched_major_frame;
  }
  cpus_cur_part[hrt_get_id()] = next_part;
  sched_proc(0);
}

void sched_proc_give_up(int round_robin) {
  struct proc *proc = cpus_cur_proc[hrt_get_id()];
  struct proc *next_proc = sched_elect_next_proc(round_robin);
  if (proc != next_proc) {
    struct context *context;
    panic_e(ctx_alloc(&context));
    hlist_insert_head(&proc->pr_trapframe.tf_ctx_list, &context->ctx_link);
    extern void _sched_proc_give_up(struct context * context, int round_robin);
    _sched_proc_give_up(context, round_robin);
  }
}

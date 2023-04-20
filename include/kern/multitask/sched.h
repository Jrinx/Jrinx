#ifndef _KERN_MULTITASK_SCHED_H_
#define _KERN_MULTITASK_SCHED_H_

#include <kern/lib/debug.h>
#include <kern/multitask/process.h>

struct sched_module {
  struct part *sm_part;
  sys_time_t sm_offset;
  sys_time_t sm_duration;
  sys_time_t sm_prev_act_time;
  struct linked_node sm_link;
};

struct sched_conf {
  const char *sc_pa_name;
  sys_time_t sc_offset;
  sys_time_t sc_duration;
};

extern struct part **cpus_cur_part;
extern struct proc **cpus_cur_proc;

void sched_init(void);
long sched_module_add(struct sched_conf *conf) __attribute__((warn_unused_result));
void sched_add_part(struct part *part);
long sched_launch(void) __attribute__((warn_unused_result));
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

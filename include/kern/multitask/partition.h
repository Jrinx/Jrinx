#ifndef _KERN_MULTITASK_PARTITION_H_
#define _KERN_MULTITASK_PARTITION_H_

#include <kern/mm/vmm.h>
#include <lib/hashmap.h>
#include <list.h>
#include <stdint.h>
#include <types.h>

struct part {
  char *pa_name;
  pte_t *pa_pgdir;
  unsigned long *pa_cpus_asid;
  uint64_t *pa_cpus_asid_generation;
  unsigned long pa_mem_req;
  unsigned long pa_mem_rem;
  unsigned long pa_ustasktop;
  unsigned long pa_xstacktop;
  unsigned long pa_entrypoint;
  unsigned long pa_main_proc_stacksize;
  sys_time_t pa_period;
  sys_time_t pa_duration;
  part_id_t pa_id;
  lock_level_t pa_lock_level;
  op_mode_t pa_op_mode;
  start_cond_t pa_start_cond;
  num_cores_t pa_num_cores;
  struct hashmap pa_proc_name_map;
  struct list_head pa_proc_list;
  struct linked_node pa_id_link;
  struct linked_node pa_sched_link;
};

struct part_conf {
  const char *pa_name;
  const char *pa_prog;
  size_t pa_mem_req;
  sys_time_t pa_period;
  sys_time_t pa_duration;
};

struct prog_def_t {
  const char *pg_name;
  const uint8_t *pg_elf_bin;
  const size_t pg_elf_size;
};

#define link_prog(name)                                                                        \
  struct prog_def_t *name##_prog __attribute__((section(".ksec.prog_def." #name))) = &name

struct part *part_from_id(part_id_t id);
void part_add_proc_name(struct part *part, struct proc *proc);
struct proc *part_get_proc_by_name(struct part *part, const char *name);
long part_alloc(struct part **part, const char *name, unsigned long memory_req,
                sys_time_t period, sys_time_t duration) __attribute__((warn_unused_result));
long part_free(struct part *part) __attribute__((warn_unused_result));
long part_pt_alloc(struct part *part, vaddr_t vaddr, perm_t perm, void **pa);
long part_create(struct part_conf *conf) __attribute__((warn_unused_result));

static inline size_t part_get_proc_count(struct part *part) {
  return part->pa_proc_name_map.h_num;
}

#endif

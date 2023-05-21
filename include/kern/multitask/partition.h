#ifndef _KERN_MULTITASK_PARTITION_H_
#define _KERN_MULTITASK_PARTITION_H_

#include <attr.h>
#include <kern/lock/lock.h>
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
  unsigned long pa_comm_base;
  unsigned long pa_entrypoint;
  unsigned long pa_main_proc_stacksize;
  sys_time_t pa_period;
  sys_time_t pa_duration;
  part_id_t pa_id;
  lock_level_t pa_lock_level;
  op_mode_t pa_op_mode;
  start_cond_t pa_start_cond;
  num_cores_t pa_num_cores;
  struct lock pa_mem_rem_lock;
  struct lock pa_comm_base_lock;
  struct hashmap pa_proc_name_map;
  struct hashmap pa_buf_name_map;
  struct hashmap pa_bb_name_map;
  struct list_head pa_proc_list;
  struct linked_node pa_id_link;
  struct linked_node pa_name_link;
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
  struct prog_def_t *name##_prog __section(".ksec.prog_def." #name) __used = &name

struct proc;
struct buffer;
struct blackboard;

struct part *part_from_id(part_id_t id);
struct part *part_from_name(const char *name);
void part_add_proc_name(struct part *part, struct proc *proc);
struct proc *part_get_proc_by_name(struct part *part, const char *name);
void part_add_buf_name(struct part *part, struct buffer *buf);
struct buffer *part_get_buf_by_name(struct part *part, const char *name);
void part_add_bb_name(struct part *part, struct blackboard *bb);
struct blackboard *part_get_bb_by_name(struct part *part, const char *name);
long part_comm_alloc(struct part *part, size_t size, void **out);
long part_pt_alloc(struct part *part, vaddr_t vaddr, perm_t perm, void **pa);
long part_create(struct part_conf *conf) __warn_unused_result;
void part_pt_sync_kern_pgdir(void);

static inline size_t part_get_proc_count(struct part *part) {
  return part->pa_proc_name_map.h_num;
}

#endif

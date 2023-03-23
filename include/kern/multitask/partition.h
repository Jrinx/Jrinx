#ifndef _KERN_MULTITASK_PARTITION_H_
#define _KERN_MULTITASK_PARTITION_H_

#include <kern/mm/vmm.h>
#include <lib/hashmap.h>
#include <list.h>
#include <stdint.h>

struct part {
  uintmax_t pa_id;
  const char *pa_name;
  pte_t *pa_pgdir;
  unsigned long *pa_cpus_asid;
  uint64_t *pa_cpus_asid_generation;
  unsigned long pa_mem_req;
  unsigned long pa_mem_rem;
  unsigned long pa_ustasktop;
  unsigned long pa_entrypoint;
  unsigned long pa_main_proc_stacksize;
  struct linked_node pa_id_link;
};

struct part_conf {
  const char *pa_name;
  const char *pa_prog;
  size_t pa_mem_req;
};

struct prog_def_t {
  const char *pg_name;
  const uint8_t *pg_elf_bin;
  const size_t pg_elf_size;
};

#define link_prog(name)                                                                        \
  struct prog_def_t *name##_prog __attribute__((section(".ksec.prog_def." #name))) = &name

struct part *part_from_id(uintmax_t id);
long part_alloc(struct part **part, const char *name, unsigned long memory_req)
    __attribute__((warn_unused_result));
long part_free(struct part *part) __attribute__((warn_unused_result));
long part_pt_alloc(struct part *part, vaddr_t vaddr, perm_t perm, void **pa);
long part_create(struct part_conf *conf) __attribute__((warn_unused_result));

#endif

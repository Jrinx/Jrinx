#include <kern/comm/buffer.h>
#include <kern/drivers/cpus.h>
#include <kern/lib/debug.h>
#include <kern/lib/logger.h>
#include <kern/lib/sync.h>
#include <kern/lock/lock.h>
#include <kern/lock/spinlock.h>
#include <kern/mm/kalloc.h>
#include <kern/mm/pmm.h>
#include <kern/multitask/partition.h>
#include <kern/multitask/process.h>
#include <kern/multitask/sched.h>
#include <layouts.h>
#include <lib/elfloader.h>
#include <lib/string.h>

static const void *part_id_key_of(const struct linked_node *node) {
  const struct part *part = CONTAINER_OF(node, struct part, pa_id_link);
  return &part->pa_id;
}

static const void *part_name_key_of(const struct linked_node *node) {
  const struct part *part = CONTAINER_OF(node, struct part, pa_name_link);
  return part->pa_name;
}

static const void *part_proc_name_key_of(const struct linked_node *node) {
  const struct proc *proc = CONTAINER_OF(node, struct proc, pr_name_link);
  return proc->pr_name;
}

static const void *part_buf_name_key_of(const struct linked_node *node) {
  const struct buffer *buf = CONTAINER_OF(node, struct buffer, buf_name_link);
  return buf->buf_name;
}

static struct hlist_head part_id_map_array[128];
static struct hashmap part_id_map = {
    .h_array = part_id_map_array,
    .h_num = 0,
    .h_cap = 128,
    .h_code = hash_code_uint64,
    .h_equals = hash_eq_uint64,
    .h_key = part_id_key_of,
};
static with_spinlock(part_id_map);

static uint64_t part_id_alloc(void) {
  static uint64_t nxt_id = 1;
  static with_spinlock(nxt_id);
  uint64_t ret;
  panic_e(lk_acquire(&spinlock_of(nxt_id)));
  ret = nxt_id;
  nxt_id++;
  panic_e(lk_release(&spinlock_of(nxt_id)));
  return ret;
}

struct part *part_from_id(part_id_t pa_id) {
  struct linked_node *node = hashmap_get(&part_id_map, &pa_id);
  if (node == NULL) {
    return NULL;
  }
  struct part *part = CONTAINER_OF(node, struct part, pa_id_link);
  return part;
}

static struct hlist_head part_name_map_array[128];
static struct hashmap part_name_map = {
    .h_array = part_name_map_array,
    .h_num = 0,
    .h_cap = 128,
    .h_code = hash_code_str,
    .h_equals = hash_eq_str,
    .h_key = part_name_key_of,
};
static with_spinlock(part_name_map);

struct part *part_from_name(const char *name) {
  struct linked_node *node = hashmap_get(&part_name_map, name);
  if (node == NULL) {
    return NULL;
  }
  struct part *part = CONTAINER_OF(node, struct part, pa_name_link);
  return part;
}

void part_add_proc_name(struct part *part, struct proc *proc) {
  hashmap_put(&part->pa_proc_name_map, &proc->pr_name_link);
}

struct proc *part_get_proc_by_name(struct part *part, const char *name) {
  struct linked_node *node = hashmap_get(&part->pa_proc_name_map, name);
  if (node == NULL) {
    return NULL;
  }
  struct proc *proc = CONTAINER_OF(node, struct proc, pr_name_link);
  return proc;
}

void part_add_buf_name(struct part *part, struct buffer *buf) {
  hashmap_put(&part->pa_buf_name_map, &buf->buf_name_link);
}

struct buffer *part_get_buf_by_name(struct part *part, const char *name) {
  struct linked_node *node = hashmap_get(&part->pa_buf_name_map, name);
  if (node == NULL) {
    return NULL;
  }
  struct buffer *buf = CONTAINER_OF(node, struct buffer, buf_name_link);
  return buf;
}

long part_alloc(struct part **part, const char *name, unsigned long memory_req,
                sys_time_t period, sys_time_t duration) {
  struct phy_frame *frame;
  catch_e(phy_frame_alloc(&frame));
  unsigned long pa;
  panic_e(frame2pa(frame, &pa));
  memcpy((void *)pa, kern_pgdir, PGSIZE);
  struct part *tmp = kalloc(sizeof(struct part));
  memset(tmp, 0, sizeof(struct part));
  size_t name_len = strlen(name);
  tmp->pa_name = kalloc((name_len + 1) * sizeof(char));
  strcpy(tmp->pa_name, name);
  tmp->pa_mem_req = memory_req;
  tmp->pa_mem_rem = memory_req;
  tmp->pa_ustasktop = USTKLIMIT;
  tmp->pa_xstacktop = XSTKLIMIT;
  tmp->pa_comm_base = COMM_BASE;
  tmp->pa_id = part_id_alloc();
  // TODO: init period, duration, num_cores
  tmp->pa_period = period;
  tmp->pa_duration = duration;
  tmp->pa_num_cores = 1;
  tmp->pa_lock_level = 0;
  tmp->pa_op_mode = COLD_START;
  tmp->pa_start_cond = NORMAL_START;
  struct hlist_head *proc_name_map_array = kalloc(sizeof(struct hlist_head) * 16);
  memset(proc_name_map_array, 0, sizeof(struct hlist_head) * 16);
  HASHMAP_ALLOC(&tmp->pa_proc_name_map, proc_name_map_array, 16, str, part_proc_name_key_of);
  struct hlist_head *buf_name_map_array = kalloc(sizeof(struct hlist_head) * 64);
  memset(buf_name_map_array, 0, sizeof(struct hlist_head) * 64);
  HASHMAP_ALLOC(&tmp->pa_buf_name_map, buf_name_map_array, 64, str, part_buf_name_key_of);
  list_init(&tmp->pa_proc_list);
  tmp->pa_cpus_asid = kalloc(sizeof(unsigned long) * cpus_get_count());
  tmp->pa_cpus_asid_generation = kalloc(sizeof(unsigned long) * cpus_get_count());
  memset(tmp->pa_cpus_asid_generation, 0, sizeof(unsigned long) * cpus_get_count());
  tmp->pa_pgdir = (pte_t *)pa;
  *part = tmp;
  panic_e(lk_acquire(&spinlock_of(part_id_map)));
  hashmap_put(&part_id_map, &tmp->pa_id_link);
  panic_e(lk_release(&spinlock_of(part_id_map)));
  panic_e(lk_acquire(&spinlock_of(part_name_map)));
  hashmap_put(&part_name_map, &tmp->pa_name_link);
  panic_e(lk_release(&spinlock_of(part_name_map)));
  return KER_SUCCESS;
}

long part_free(struct part *part) {
  // TODO
  return KER_SUCCESS;
}

long part_pt_alloc(struct part *part, vaddr_t vaddr, perm_t perm, void **pa) {
  if (part->pa_mem_rem < PGSIZE) {
    return -KER_PART_ER;
  }
  struct phy_frame *frame;
  catch_e(phy_frame_alloc(&frame));
  paddr_t paddr;
  catch_e(frame2pa(frame, &paddr.val));
  *pa = (void *)paddr.val;
  catch_e(pt_map(part->pa_pgdir, vaddr, paddr, perm), {
    panic_e(phy_frame_ref_dec(frame));
    return err;
  });
  part->pa_mem_rem -= PGSIZE;
  return KER_SUCCESS;
}

static struct prog_def_t *prog_find_by_name(const char *prog_name) {
  extern struct prog_def_t *kern_prog_def_begin[];
  extern struct prog_def_t *kern_prog_def_end[];
  for (struct prog_def_t **ptr = kern_prog_def_begin; ptr < kern_prog_def_end; ptr++) {
    struct prog_def_t *prog_def = *ptr;
    if (strcmp(prog_def->pg_name, prog_name) == 0) {
      return prog_def;
    }
  }
  return NULL;
}

struct part_load_prog_mapper_ctx {
  struct part *part;
  Elf64_Phdr *phdr;
};

static long part_load_prog_mapper(void *data, unsigned long va, size_t offset, const void *src,
                                  size_t src_len) {
  struct part_load_prog_mapper_ctx *ctx = data;
  void *pa;
  vaddr_t vaddr = {.val = va};
  perm_t perm = {.bits = {.v = 1, .u = 1}};
  if (ctx->phdr->p_flags & PF_X) {
    perm.bits.x = 1;
  }
  if (ctx->phdr->p_flags & PF_W) {
    perm.bits.w = 1;
  }
  if (ctx->phdr->p_flags & PF_R) {
    perm.bits.r = 1;
  }
  catch_e(part_pt_alloc(ctx->part, vaddr, perm, &pa));
  if (src != NULL) {
    memcpy(pa + offset, src, src_len);
  }
  return KER_SUCCESS;
}

static long part_load_prog(struct part *part, struct prog_def_t *prog) {
  const Elf64_Ehdr *ehdr = elf_from(prog->pg_elf_bin, prog->pg_elf_size);
  if (ehdr == NULL) {
    return -KER_ELF_ER;
  }
  ELF_PHDR_ITER (ehdr, phdr) {
    struct part_load_prog_mapper_ctx ctx = {.part = part, .phdr = phdr};
    cb_decl(elf_mapper_callback_t, part_load_prog_callback, part_load_prog_mapper, &ctx);
    if (phdr->p_type == PT_LOAD) {
      catch_e(elf_load_prog(phdr, prog->pg_elf_bin + phdr->p_offset, part_load_prog_callback));
    }
  }
  fence_i;
  const char *strtab = NULL;
  ELF_SHDR_ITER (ehdr, shdr) {
    if (shdr->sh_type == SHT_STRTAB) {
      strtab = (char *)(prog->pg_elf_bin + shdr->sh_offset);
      break;
    }
  }
  if (strtab == NULL) {
    return -KER_ELF_ER;
  }
  unsigned long main_proc_stacktop = 0;
  ELF_SHDR_ITER (ehdr, shdr) {
    if (shdr->sh_type == SHT_SYMTAB) {
      Elf64_Sym *symtab = (Elf64_Sym *)(prog->pg_elf_bin + shdr->sh_offset);
      for (size_t i = 0; i < shdr->sh_size / shdr->sh_entsize; i++) {
        if (strcmp(strtab + symtab[i].st_name, STR(UMAINSTKTOP_SYM)) == 0) {
          main_proc_stacktop = symtab[i].st_value;
          break;
        }
      }
    }
  }
  if (main_proc_stacktop > USTKLIMIT) {
    return -KER_ELF_ER;
  }
  part->pa_main_proc_stacksize = USTKLIMIT - main_proc_stacktop;
  part->pa_entrypoint = ehdr->e_entry;
  return KER_SUCCESS;
}

long part_create(struct part_conf *conf) {
  info("create partition: name='%s',prog='%s',memory=%pB,period=%lu us,duration=%lu us\n",
       conf->pa_name, conf->pa_prog, &conf->pa_mem_req, conf->pa_period, conf->pa_duration);
  struct prog_def_t *prog_def = prog_find_by_name(conf->pa_prog);
  if (prog_def == NULL) {
    return -KER_PART_ER;
  }
  struct fmt_mem_range mem_range = {.addr = (unsigned long)prog_def->pg_elf_bin,
                                    .size = prog_def->pg_elf_size};
  info("program '%s' found at %pM (size: %pB)\n", conf->pa_prog, &mem_range,
       &prog_def->pg_elf_size);
  struct part *part;
  catch_e(
      part_alloc(&part, conf->pa_name, conf->pa_mem_req, conf->pa_period, conf->pa_duration));
  panic_e(part_load_prog(part, prog_def));
  info("create main process for partition '%s': name='main',entrypoint=%016lx,stacksize=%pB\n",
       part->pa_name, part->pa_entrypoint, &part->pa_main_proc_stacksize);
  struct proc *main_proc;
  catch_e(proc_alloc(part, &main_proc, "main", 0, 0, part->pa_entrypoint,
                     part->pa_main_proc_stacksize, 0, SOFT));
  main_proc->pr_state = READY;
  info("remaining memory of partition '%s': %pB\n", part->pa_name, &part->pa_mem_rem);
  sched_add_part(part);
  return KER_SUCCESS;
}

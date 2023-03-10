#include <kern/lib/debug.h>
#include <kern/lib/logger.h>
#include <kern/lock/lock.h>
#include <kern/lock/spinlock.h>
#include <kern/mm/asid.h>
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

struct part *part_from_id(uint64_t pa_id) {
  struct linked_node *node = hashmap_get(&part_id_map, &pa_id);
  if (node == NULL) {
    return NULL;
  }
  struct part *part = CONTAINER_OF(node, struct part, pa_id_link);
  return part;
}

long part_alloc(struct part **part, const char *name, unsigned long memory_req) {
  struct phy_frame *frame;
  catch_e(phy_frame_alloc(&frame));
  unsigned long pa;
  panic_e(frame2pa(frame, &pa));
  memcpy((void *)pa, kern_pgdir, PGSIZE);
  struct part *tmp = alloc(sizeof(struct part), sizeof(struct part));
  memset(tmp, 0, sizeof(struct part));
  tmp->pa_name = name;
  tmp->pa_mem_req = memory_req;
  tmp->pa_mem_rem = memory_req;
  tmp->pa_ustasktop = USTKLIMIT;
  tmp->pa_id = part_id_alloc();
  catch_e(asid_alloc(&tmp->pa_asid));
  tmp->pa_pgdir = (pte_t *)pa;
  *part = tmp;
  panic_e(lk_acquire(&spinlock_of(part_id_map)));
  hashmap_put(&part_id_map, &tmp->pa_id_link);
  panic_e(lk_release(&spinlock_of(part_id_map)));
  return KER_SUCCESS;
}

long part_free(struct part *part) {
  // TODO
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
  struct elf_phdr *phdr;
};

static long part_load_prog_mapper(void *data, unsigned long va, size_t offset, const void *src,
                                  size_t src_len) {
  struct part_load_prog_mapper_ctx *ctx = data;
  if (ctx->part->pa_mem_rem < PGSIZE) {
    return -KER_PART_ER;
  }
  struct phy_frame *frame;
  catch_e(phy_frame_alloc(&frame));
  unsigned long pa;
  catch_e(frame2pa(frame, &pa));
  if (src != NULL) {
    memcpy((void *)pa + offset, src, src_len);
  }
  vaddr_t vaddr = {.val = va};
  paddr_t paddr = {.val = pa};
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
  catch_e(pt_map(ctx->part->pa_pgdir, vaddr, paddr, perm), {
    panic_e(phy_frame_ref_dec(frame));
    return err;
  });
  ctx->part->pa_mem_rem -= PGSIZE;
  return KER_SUCCESS;
}

static long part_load_prog(struct part *part, struct prog_def_t *prog) {
  const struct elf_ehdr *ehdr = elf_from(prog->pg_elf_bin, prog->pg_elf_size);
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
  part->pa_entrypoint = ehdr->e_entry;
  return KER_SUCCESS;
}

long part_create(struct part_conf *conf) {
  info("create partition: name='%s',prog='%s',memory=%pB\n", conf->pa_name, conf->pa_prog,
       &conf->pa_mem_req);
  struct prog_def_t *prog_def = prog_find_by_name(conf->pa_prog);
  if (prog_def == NULL) {
    return -KER_PART_ER;
  }
  struct fmt_mem_range mem_range = {.addr = (unsigned long)prog_def->pg_elf_bin,
                                    .size = prog_def->pg_elf_size};
  info("program '%s' found at %pM (size: %pB)\n", conf->pa_prog, &mem_range,
       &prog_def->pg_elf_size);
  struct part *part;
  catch_e(part_alloc(&part, conf->pa_name, conf->pa_mem_req));
  panic_e(part_load_prog(part, prog_def));
  size_t stacksize_default = USTKSIZE_DEFAULT;
  info("create main process for partition '%s': name='main',entrypoint=%016lx,stacksize=%pB\n",
       part->pa_name, part->pa_entrypoint, &stacksize_default);
  struct proc *main_proc;
  catch_e(proc_alloc(part, &main_proc, "main", part->pa_entrypoint, USTKSIZE_DEFAULT));
  info("remaining memory of partition '%s': %pB\n", part->pa_name, &part->pa_mem_rem);

  // TODO assign to different processors
  catch_e(sched_assign_proc(SYSCORE, main_proc));
  return KER_SUCCESS;
}

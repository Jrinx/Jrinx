#include <aligns.h>
#include <kern/drivers/cpus.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lib/sbi.h>
#include <kern/lib/sync.h>
#include <kern/lock/lock.h>
#include <kern/lock/spinlock.h>
#include <kern/mm/asid.h>
#include <kern/mm/pmm.h>
#include <kern/multitask/partition.h>
#include <kern/multitask/process.h>
#include <kern/traps/traps.h>
#include <layouts.h>
#include <lib/string.h>

static const void *proc_id_key_of(const struct linked_node *node) {
  const struct proc *proc = CONTAINER_OF(node, struct proc, pr_id_link);
  return &proc->pr_id;
}

static struct hlist_head proc_id_map_array[128];
static struct hashmap proc_id_map = {
    .h_array = proc_id_map_array,
    .h_num = 0,
    .h_cap = 128,
    .h_code = hash_code_uint64,
    .h_equals = hash_eq_uint64,
    .h_key = proc_id_key_of,
};
static with_spinlock(proc_id_map);

static uint64_t proc_id_alloc(void) {
  static uint64_t nxt_id = 1;
  static with_spinlock(nxt_id);
  uint64_t ret;
  panic_e(lk_acquire(&spinlock_of(nxt_id)));
  ret = nxt_id;
  nxt_id++;
  panic_e(lk_release(&spinlock_of(nxt_id)));
  return ret;
}

static long proc_stack_setup(struct part *part, struct proc *proc, unsigned long stacktop,
                             size_t stacksize) {
  proc->pr_ustacktop = stacktop;
  proc->pr_ustacksize = stacksize;
  for (size_t i = 0; i < stacksize; i += PGSIZE) {
    struct phy_frame *frame;
    catch_e(phy_frame_alloc(&frame));
    unsigned long pa;
    panic_e(frame2pa(frame, &pa));
    vaddr_t vaddr = {.val = stacktop - i - PGSIZE};
    paddr_t paddr = {.val = pa};
    perm_t perm = {.bits = {.r = 1, .u = 1, .v = 1, .w = 1}};
    catch_e(pt_map(part->pa_pgdir, vaddr, paddr, perm));
  }
  return KER_SUCCESS;
}

long proc_alloc(struct part *part, struct proc **proc, const char *name,
                unsigned long entrypoint, size_t stacksize) {
  size_t aligned_stacksize = align_up(stacksize, PGSIZE);
  if (part->pa_mem_rem < aligned_stacksize) {
    return -KER_PROC_ER;
  }
  struct proc *tmp = alloc(sizeof(struct proc), sizeof(struct proc));
  memset(tmp, 0, sizeof(struct proc));
  struct context *proc_ctx = alloc(sizeof(struct context), sizeof(struct context));
  memset(proc_ctx, 0, sizeof(struct context));
  proc_ctx->ctx_sepc = entrypoint;
  proc_ctx->ctx_hartid = HARTID_MAX;
  rv64_si *proc_ctx_sie = (rv64_si *)&proc_ctx->ctx_sie;
  proc_ctx_sie->bits.sei = 1;
  proc_ctx_sie->bits.ssi = 1;
  proc_ctx_sie->bits.sti = 1;
  rv64_sstatus *proc_ctx_sstatus = (rv64_sstatus *)&proc_ctx->ctx_sstatus;
  proc_ctx_sstatus->bits.spie = 1;
  proc_ctx_sstatus->bits.spp = RISCV_U_MODE;
  proc_ctx_sstatus->bits.sum = 1;
  hlist_insert_head(&tmp->pr_trapframe.tf_ctx_list, &proc_ctx->ctx_link);
  tmp->pr_name = name;
  tmp->pr_entrypoint = entrypoint;
  tmp->pr_id = proc_id_alloc();
  tmp->pr_part_id = part->pa_id;
  proc_stack_setup(part, tmp, part->pa_ustasktop, aligned_stacksize);
  part->pa_ustasktop -= aligned_stacksize + PGSIZE;
  part->pa_mem_rem -= aligned_stacksize;
  *proc = tmp;
  panic_e(lk_acquire(&spinlock_of(proc_id_map)));
  hashmap_put(&proc_id_map, &tmp->pr_id_link);
  panic_e(lk_release(&spinlock_of(proc_id_map)));
  return KER_SUCCESS;
}

long proc_free(struct proc *proc) {
  return KER_SUCCESS;
}

void proc_run(struct proc *proc) {
  struct context *proc_top_ctx =
      CONTAINER_OF(proc->pr_trapframe.tf_ctx_list.h_first, struct context, ctx_link);
  cpus_context[hrt_get_id()] = proc_top_ctx; // TODO: free previous context
  hlist_remove_node(&proc_top_ctx->ctx_link);
  proc_top_ctx->ctx_hartid = hrt_get_id();
  struct part *part = part_from_id(proc->pr_part_id);
  if (part == NULL) {
    fatal("unknown partition id: %lu\n", proc->pr_part_id);
  }
  if (part->pa_cpus_asid[hrt_get_id()] > asid_get_max()) {
    panic_e(asid_alloc(&part->pa_cpus_asid[hrt_get_id()])); // TODO: asid generation
  }
  rv64_satp proc_satp = {.bits = {.mode = SV39,
                                  .asid = part->pa_cpus_asid[hrt_get_id()],
                                  .ppn = ((unsigned long)part->pa_pgdir) / PGSIZE}};
  csrw_satp(proc_satp.val);
  sfence_vma_asid(part->pa_cpus_asid[hrt_get_id()]);
  extern int args_debug_as_switch;
  if (args_debug_as_switch) {
    info("switch to address space of '%s' (asid: %lu)\n", part->pa_name,
         part->pa_cpus_asid[hrt_get_id()]);
  }
  // TODO: set timer through 'period' field?
  panic_e(sbi_set_timer(r_time() + cpus_get_timebase_freq() / 100));
  extern void trap_ret(struct context * ctx) __attribute__((noreturn));
  trap_ret(proc_top_ctx);
}

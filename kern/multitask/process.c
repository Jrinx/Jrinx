#include <aligns.h>
#include <kern/drivers/cpus.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lib/sbi.h>
#include <kern/lib/sync.h>
#include <kern/lock/lock.h>
#include <kern/lock/spinlock.h>
#include <kern/mm/asid.h>
#include <kern/mm/kalloc.h>
#include <kern/mm/pmm.h>
#include <kern/multitask/partition.h>
#include <kern/multitask/process.h>
#include <kern/traps/timer.h>
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

struct proc *proc_from_id(proc_id_t pr_id) {
  struct linked_node *node = hashmap_get(&proc_id_map, &pr_id);
  if (node == NULL) {
    return NULL;
  }
  struct proc *proc = CONTAINER_OF(node, struct proc, pr_id_link);
  return proc;
}

static long proc_ustack_setup(struct part *part, struct proc *proc, size_t stacksize) {
  proc->pr_ustacktop = part->pa_ustasktop;
  proc->pr_ustacksize = stacksize;
  for (size_t i = 0; i < stacksize; i += PGSIZE) {
    struct phy_frame *frame;
    catch_e(phy_frame_alloc(&frame));
    unsigned long pa;
    panic_e(frame2pa(frame, &pa));
    vaddr_t vaddr = {.val = part->pa_ustasktop - i - PGSIZE};
    paddr_t paddr = {.val = pa};
    perm_t perm = {.bits = {.r = 1, .u = 1, .v = 1, .w = 1}};
    catch_e(pt_map(part->pa_pgdir, vaddr, paddr, perm));
  }
  part->pa_ustasktop -= stacksize + PGSIZE;
  part->pa_mem_rem -= stacksize;
  return KER_SUCCESS;
}

static long proc_xstack_setup(struct part *part, struct proc *proc) {
  proc->pr_trapframe.tf_xstacktop = part->pa_xstacktop;
  for (size_t i = 0; i < KSTKSIZE; i += PGSIZE) {
    struct phy_frame *frame;
    catch_e(phy_frame_alloc(&frame));
    unsigned long pa;
    panic_e(frame2pa(frame, &pa));
    vaddr_t vaddr = {.val = part->pa_xstacktop - i - PGSIZE};
    paddr_t paddr = {.val = pa};
    perm_t perm = {.bits = {.a = 1, .d = 1, .r = 1, .v = 1, .w = 1}};
    catch_e(pt_map(part->pa_pgdir, vaddr, paddr, perm));
  }
  part->pa_xstacktop -= KSTKSIZE + PGSIZE;
  part->pa_mem_rem -= KSTKSIZE;
  return KER_SUCCESS;
}

void proc_reset(struct proc *proc) {
  while (!hlist_empty(&proc->pr_trapframe.tf_ctx_list)) {
    struct linked_node *node = proc->pr_trapframe.tf_ctx_list.h_first;
    struct context *ctx = CONTAINER_OF(node, struct context, ctx_link);
    hlist_remove_node(node);
    ctx_free(ctx);
  }
  struct context *proc_ctx;
  panic_e(ctx_alloc(&proc_ctx));
  memset(proc_ctx, 0, sizeof(struct context));
  proc_ctx->ctx_sepc = proc->pr_entrypoint;
  proc_ctx->ctx_hartid = HARTID_MAX;
  *(rv64_si *)&proc_ctx->ctx_sie = (rv64_si){.bits = {
                                                 .sei = 1,
                                                 .ssi = 1,
                                                 .sti = 1,
                                             }};
  *(rv64_sstatus *)&proc_ctx->ctx_sstatus = (rv64_sstatus){.bits = {
                                                               .fs = 1,
                                                               .spie = 1,
                                                               .spp = RISCV_U_MODE,
                                                               .sum = 1,
                                                           }};
  proc_ctx->ctx_regs.names.sp = proc->pr_ustacktop;
  hlist_insert_head(&proc->pr_trapframe.tf_ctx_list, &proc_ctx->ctx_link);
  spinlock_init(&proc->pr_state_lock);
}

long proc_alloc(struct part *part, struct proc **proc, const char *name, sys_time_t period,
                sys_time_t time_cap, sys_addr_t entrypoint, stack_size_t stacksize,
                priority_t base_pri, deadline_t deadline) {
  size_t aligned_stacksize = align_up(stacksize, PGSIZE);
  if (part->pa_mem_rem < aligned_stacksize) {
    return -KER_PROC_ER;
  }
  struct proc *tmp = kalloc(sizeof(struct proc));
  memset(tmp, 0, sizeof(struct proc));
  size_t name_len = strlen(name);
  tmp->pr_name = kalloc((name_len + 1) * sizeof(char));
  strcpy(tmp->pr_name, name);
  tmp->pr_id = proc_id_alloc();
  tmp->pr_idx = part_get_proc_count(part);
  tmp->pr_part_id = part->pa_id;
  // TODO: init base_pri, period, time_cap, deadline, cur_pri, deadline_time, state, core_id
  tmp->pr_period = period;
  tmp->pr_time_cap = time_cap;
  tmp->pr_entrypoint = entrypoint;
  panic_e(proc_ustack_setup(part, tmp, aligned_stacksize));
  panic_e(proc_xstack_setup(part, tmp));
  tmp->pr_base_pri = base_pri;
  tmp->pr_deadline = deadline;
  tmp->pr_cur_pri = 0;
  tmp->pr_deadline_time = 0;
  tmp->pr_state = DORMANT;
  tmp->pr_core_id = SYSCORE;
  part_add_proc_name(part, tmp);
  list_insert_tail(&part->pa_proc_list, &tmp->pr_sched_link);
  *proc = tmp;
  panic_e(lk_acquire(&spinlock_of(proc_id_map)));
  hashmap_put(&proc_id_map, &tmp->pr_id_link);
  panic_e(lk_release(&spinlock_of(proc_id_map)));
  proc_reset(tmp);
  return KER_SUCCESS;
}

long proc_free(struct proc *proc) {
  return KER_SUCCESS;
}

void proc_run(struct proc *proc) {
  struct context *proc_top_ctx =
      CONTAINER_OF(proc->pr_trapframe.tf_ctx_list.h_first, struct context, ctx_link);
  ctx_free(cpus_context[hrt_get_id()]);
  cpus_context[hrt_get_id()] = proc_top_ctx;
  hlist_remove_node(&proc_top_ctx->ctx_link);
  proc_top_ctx->ctx_hartid = hrt_get_id();
  struct part *part = part_from_id(proc->pr_part_id);
  if (part == NULL) {
    fatal("unknown partition id: %lu\n", proc->pr_part_id);
  }
  if (part->pa_cpus_asid_generation[hrt_get_id()] < asid_get_generation()) {
    while (asid_alloc(&part->pa_cpus_asid[hrt_get_id()]) != KER_SUCCESS) {
      asid_inc_generation();
    }
    part->pa_cpus_asid_generation[hrt_get_id()] = asid_get_generation();
  }
  rv64_satp proc_satp = {.bits = {.mode = SV39,
                                  .asid = part->pa_cpus_asid[hrt_get_id()],
                                  .ppn = ((unsigned long)part->pa_pgdir) / PGSIZE}};
  extern int args_debug_as_switch;
  if (args_debug_as_switch) {
    info("switch to address space of '%s' (asid: %lu)\n", part->pa_name,
         part->pa_cpus_asid[hrt_get_id()]);
  }
  extern void trap_ret_switch_as(struct context * ctx, unsigned long satp) __noreturn;
  trap_ret_switch_as(proc_top_ctx, proc_satp.val);
}

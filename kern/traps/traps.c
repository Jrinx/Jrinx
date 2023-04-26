#include <kern/drivers/cpus.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lock/lock.h>
#include <kern/lock/spinlock.h>
#include <kern/mm/kalloc.h>
#include <kern/multitask/sched.h>
#include <kern/traps/traps.h>
#include <lib/string.h>
#include <list.h>

struct context **cpus_context;

static struct context traps_context[NCTX_MAX];
static struct hlist_head traps_ctx_free_list;
static with_spinlock(traps_ctx_free_list);

long ctx_alloc(struct context **ctx) {
  panic_e(lk_acquire(&spinlock_of(traps_ctx_free_list)));
  if (hlist_empty(&traps_ctx_free_list)) {
    panic_e(lk_release(&spinlock_of(traps_ctx_free_list)));
    return -KER_CTX_ER;
  }
  struct linked_node *node = hlist_first(&traps_ctx_free_list);
  struct context *p = CONTAINER_OF(node, struct context, ctx_link);
  hlist_remove_node(node);
  panic_e(lk_release(&spinlock_of(traps_ctx_free_list)));
  *ctx = p;
  return KER_SUCCESS;
}

void ctx_free(struct context *ctx) {
  panic_e(lk_acquire(&spinlock_of(traps_ctx_free_list)));
  hlist_insert_head(&traps_ctx_free_list, &ctx->ctx_link);
  panic_e(lk_release(&spinlock_of(traps_ctx_free_list)));
}

void traps_init(void) {
  for (size_t i = 0; i < NCTX_MAX; i++) {
    hlist_insert_head(&traps_ctx_free_list, &traps_context[i].ctx_link);
  }
  cpus_context = kalloc(sizeof(struct context *) * cpus_get_count());
  for (size_t i = 0; i < cpus_get_count(); i++) {
    panic_e(ctx_alloc(&cpus_context[i]));
    memset(cpus_context[i], 0, sizeof(struct context));
    cpus_context[i]->ctx_hartid = i;
  }
}

void trap_init_vec(void) {
  extern void trap_vec(void);
  rv64_stvec stvec_reg = {.bits = {.mode = DIRECT, .base = (unsigned long)trap_vec / 4}};
  csrw_stvec(stvec_reg.val);
  csrw_sscratch((unsigned long)cpus_context[hrt_get_id()]);
}

static void prepare_nested_trap(void) {
  struct context *context = cpus_context[hrt_get_id()];
  struct proc *proc = sched_cur_proc();
  hlist_insert_head(&proc->pr_trapframe.tf_ctx_list, &context->ctx_link);
  panic_e(ctx_alloc(&cpus_context[hrt_get_id()]));
  cpus_context[hrt_get_id()]->ctx_hartid = hrt_get_id();
  csrw_sscratch((unsigned long)cpus_context[hrt_get_id()]);
}

void intp_enable(void) {
  rv64_sstatus sstatus = {.val = csrr_sstatus()};
  sstatus.bits.sie = 1;
  csrw_sstatus(sstatus.val);
}

void intp_disable(void) {
  rv64_sstatus sstatus = {.val = csrr_sstatus()};
  sstatus.bits.sie = 0;
  csrw_sstatus(sstatus.val);
}

void intp_pop(void) {
  if (likely(cpus_intp_layer != NULL)) {
    cpus_intp_layer[hrt_get_id()]--;
    if (cpus_intp_layer[hrt_get_id()] == 0 && cpus_retained_intp[hrt_get_id()]) {
      intp_enable();
    }
  }
}

void intp_push(void) {
  if (likely(cpus_intp_layer != NULL)) {
    rv64_sstatus sstatus = {.val = csrr_sstatus()};
    if (cpus_intp_layer[hrt_get_id()] == 0) {
      cpus_retained_intp[hrt_get_id()] = sstatus.bits.sie;
    }
    cpus_intp_layer[hrt_get_id()]++;
    intp_disable();
  }
}

extern void do_pagefault(struct context *context);
extern void do_syscall(struct context *context);
extern void do_timer_int(struct context *context);
extern void do_external_int(struct context *context);

void handle_trap(void) {
  struct context *context = cpus_context[hrt_get_id()];
  prepare_nested_trap();
  if (!(context->ctx_scause & CAUSE_INT_OFFSET)) {
    intp_enable();
  }
  extern int args_debug_kalloc_used;
  if (args_debug_kalloc_used) {
    size_t kalloc_used = kalloc_get_used();
    info("kalloc used: %pB\n", &kalloc_used);
  }
  switch (context->ctx_scause) {
  case CAUSE_EXC_IF_PAGE_FAULT:
  case CAUSE_EXC_LD_PAGE_FAULT:
  case CAUSE_EXC_ST_PAGE_FAULT:
    do_pagefault(context);
    break;
  case CAUSE_EXC_U_ECALL:
    do_syscall(context);
    break;
  case CAUSE_INT_OFFSET | CAUSE_INT_S_TIMER:
    do_timer_int(context);
    break;
  case CAUSE_INT_OFFSET | CAUSE_INT_S_EXTERNAL:
    do_external_int(context);
    break;
  default:
    fatal("failed to handle trap, cause=%016lx\n", context->ctx_scause);
    break;
  }
  if (!(context->ctx_scause & CAUSE_INT_OFFSET)) {
    intp_disable();
  }
  ctx_free(cpus_context[hrt_get_id()]);
  hlist_remove_node(&context->ctx_link);
  cpus_context[hrt_get_id()] = context;

  extern void trap_ret(struct context * context) __attribute__((noreturn));
  trap_ret(context);
}

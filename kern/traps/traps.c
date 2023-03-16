#include <kern/drivers/cpus.h>
#include <kern/lib/debug.h>
#include <kern/mm/kalloc.h>
#include <kern/multitask/sched.h>
#include <kern/traps/traps.h>
#include <lib/string.h>

struct context **cpus_context;

void traps_init(void) {
  cpus_context = kalloc(sizeof(struct context *) * cpus_get_count());
  for (size_t i = 0; i < cpus_get_count(); i++) {
    cpus_context[i] = kalloc(sizeof(struct context));
    memset(cpus_context[i], 0, sizeof(struct context));
  }
}

void trap_init_vec(void) {
  extern void trap_vec(void);
  rv64_stvec stvec_reg = {.bits = {.mode = DIRECT, .base = (unsigned long)trap_vec / 4}};
  csrw_stvec(stvec_reg.val);
}

static void prepare_nested_trap(void) {
  struct context *context = cpus_context[hrt_get_id()];
  struct proc *proc = cpus_cur_proc[hrt_get_id()];
  hlist_insert_head(&proc->pr_trapframe.tf_ctx_list, &context->ctx_link);
  cpus_context[hrt_get_id()] = kalloc(sizeof(struct context));
  memset(cpus_context[hrt_get_id()], 0, sizeof(struct context));
  cpus_context[hrt_get_id()]->ctx_hartid = hrt_get_id();
  csrw_sscratch((unsigned long)cpus_context[hrt_get_id()]);
}

extern void do_pagefault(struct context *context);
extern void do_timer_int(struct context *context);
extern void do_syscall(struct context *context);

void handle_trap(void) {
  struct context *context = cpus_context[hrt_get_id()];
  prepare_nested_trap();
  size_t kalloc_used = kalloc_get_used();
  extern int args_debug_kalloc_used;
  if (args_debug_kalloc_used) {
    info("kalloc used: %pB\n", &kalloc_used);
  }
  // Future: enable interrupt here
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
  default:
    fatal("failed to handle trap, cause=%016lx\n", context->ctx_scause);
    break;
  }
  // Future: disable interrupt here
  kfree(cpus_context[hrt_get_id()]);
  hlist_remove_node(&context->ctx_link);
  cpus_context[hrt_get_id()] = context;

  extern void trap_ret(struct context * context) __attribute__((noreturn));
  trap_ret(context);
}
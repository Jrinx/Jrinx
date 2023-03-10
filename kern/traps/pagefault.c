#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lib/hart.h>
#include <kern/lib/regs.h>
#include <kern/lib/sync.h>
#include <kern/mm/vmm.h>
#include <kern/multitask/partition.h>
#include <kern/multitask/sched.h>
#include <kern/traps/traps.h>

void do_pagefault(struct context *context) {
  const struct proc *proc = cpus_cur_proc[hrt_get_id()];
  const struct part *part = part_from_id(proc->pr_part_id);
  if (part == NULL) {
    fatal("unknown partition id: %lu\n", proc->pr_part_id);
  }
  vaddr_t va = {.val = context->ctx_stval};
  pte_t *pte;
  panic_e(pt_lookup(part->pa_pgdir, va, &pte));
  if (pte == NULL) {
    info("pagefault from illegal va=%016lx\n", va.val);
    return;
  }
  if (pte->bits.a == 0) {
    pte->bits.a = 1;
  }
  if (pte->bits.d == 0 && context->ctx_scause == CAUSE_EXC_ST_PAGE_FAULT) {
    pte->bits.d = 1;
  }
  sfence_vma_va_asid(va.val, part->pa_asid);
}

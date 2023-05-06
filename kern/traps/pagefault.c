#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lib/hart.h>
#include <kern/lib/regs.h>
#include <kern/lib/sync.h>
#include <kern/mm/pmm.h>
#include <kern/mm/vmm.h>
#include <kern/multitask/partition.h>
#include <kern/multitask/sched.h>
#include <kern/traps/traps.h>

void do_pagefault(struct context *context) {
  struct part *part = sched_cur_part();
  vaddr_t va = {.val = context->ctx_stval};
  pte_t *pte;
re_lookup:
  panic_e(pt_lookup(part->pa_pgdir, va, &pte));
  if (pte == NULL) {
    rv64_sstatus sstatus = {.val = context->ctx_sstatus};
    if (sstatus.bits.spp == RISCV_S_MODE && va.val >= COMM_BASE && va.val < COMM_LIMT) {
      perm_t perm = {.bits = {.r = 1, .w = 1}};
      catch_e(part_pt_alloc(part, va, perm, NULL), { goto err; });
    } else {
      goto err;
    }
    goto re_lookup;
  }
  if (pte->bits.a == 0) {
    pte->bits.a = 1;
  }
  if (pte->bits.d == 0 && context->ctx_scause == CAUSE_EXC_ST_PAGE_FAULT) {
    pte->bits.d = 1;
  }
  sfence_vma_va_asid(va.val, part->pa_cpus_asid[hrt_get_id()]);
  return;
err:
  fatal("pagefault from part#%lu illegal va=%016lx, epc=%016lx\n", part->pa_id, va.val,
        context->ctx_sepc);
}

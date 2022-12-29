#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lib/regs.h>
#include <kern/lib/sync.h>
#include <kern/mm/mem.h>
#include <kern/mm/vm.h>
#include <layouts.h>
#include <lib/string.h>
#include <stddef.h>

pte_t kern_pgdir[PGSIZE / sizeof(pte_t)] __attribute__((aligned(PGSIZE)));

static long pt_walk(pte_t *pgdir, vaddr_t va, int create, pte_t **pte) {
  struct phy_frame *frame1 = NULL;
  struct phy_frame *frame2 = NULL;
  pte_t *ptr = pgdir + va.bits.vpn2;
  if (!ptr->bits.v) {
    if (!create) {
      *pte = NULL;
      return KER_SUCCESS;
    }
    catch_e(phy_frame_alloc(&frame1));
    panic_e(phy_frame_ref_inc(frame1));
    paddr_t pa;
    panic_e(frame2pa(frame1, &pa.val));
    ptr->val = 0;
    ptr->bits.v = 1;
    ptr->pp.ppn = pa.pp.ppn;
  }

  ptr = (pte_t *)pte2pa(*ptr).val + va.bits.vpn1;
  if (!ptr->bits.v) {
    if (!create) {
      *pte = NULL;
      return KER_SUCCESS;
    }
    catch_e(phy_frame_alloc(&frame2), {
      if (frame1 != NULL) {
        panic_e(phy_frame_ref_dec(frame1));
      }
      return err;
    });
    panic_e(phy_frame_ref_inc(frame2));
    paddr_t pa;
    panic_e(frame2pa(frame2, &pa.val));
    ptr->val = 0;
    ptr->bits.v = 1;
    ptr->pp.ppn = pa.pp.ppn;
  }

  *pte = (pte_t *)pte2pa(*ptr).val + va.bits.vpn0;
  return KER_SUCCESS;
}

long pt_lookup(pte_t *pgdir, vaddr_t va, pte_t **res) {
  pte_t *pte;
  catch_e(pt_walk(pgdir, va, 0, &pte));
  if (pte == NULL || !pte->bits.v) {
    *res = NULL;
    return KER_SUCCESS;
  }
  *res = pte;
  return KER_SUCCESS;
}

long pt_unmap(pte_t *pgdir, vaddr_t va) {
  pte_t *pte;
  catch_e(pt_lookup(pgdir, va, &pte));

  if (pte == NULL) {
    return KER_SUCCESS;
  }

  pte->val = 0;
  paddr_t pa = pte2pa(*pte);
  struct phy_frame *frame;
  catch_e(pa2frame(pa.val, &frame), {
    // pa locates at mmio region
    return KER_SUCCESS;
  });
  catch_e(phy_frame_ref_dec(frame));
  return KER_SUCCESS;
}

long pt_map(pte_t *pgdir, vaddr_t va, paddr_t pa, perm_t perm) {
  pte_t *pte;
  va.pp.off = 0;
  pa.pp.off = 0;
  catch_e(pt_walk(pgdir, va, 0, &pte));
  if (pte != NULL && pte->bits.v) {
    catch_e(pt_unmap(pgdir, va));
  }
  catch_e(pt_walk(pgdir, va, 1, &pte));
  pte->pp.ppn = pa.pp.ppn;
  perm.bits.a = 1;
  perm.bits.d = 1;
  perm.bits.v = 1;
  pte->pp.perm = perm.val;

  struct phy_frame *frame;
  catch_e(pa2frame(pa.val, &frame), {
    // pa locates at mmio region
    return KER_SUCCESS;
  });

  panic_e(phy_frame_ref_inc(frame));
  return KER_SUCCESS;
}

void vm_init_kern_pgdir(void) {
  extern uint8_t kern_text_end[];
  vaddr_t va = {.val = KERNBASE};
  paddr_t pa = {.val = KERNBASE};
  size_t text_end = (size_t)kern_text_end / PGSIZE * PGSIZE + PGSIZE;
  size_t free_end = (size_t)mm_get_freemem_base() / PGSIZE * PGSIZE + PGSIZE;

  info("set up kernel text mapping at ");
  mm_print_range(KERNBASE, text_end - KERNBASE, NULL);
  for (; va.val < text_end; va.val += PGSIZE, pa.val += PGSIZE) {
    perm_t perm = {.bits = {.a = 1, .d = 1, .r = 1, .x = 1, .w = 1, .g = 1}};
    panic_e(pt_map(kern_pgdir, va, pa, perm));
  }

  info("set up kernel data mapping at ");
  mm_print_range(text_end, free_end - text_end, NULL);
  for (; va.val < free_end; va.val += PGSIZE, pa.val += PGSIZE) {
    perm_t perm = {.bits = {.a = 1, .d = 1, .r = 1, .w = 1, .g = 1}};
    panic_e(pt_map(kern_pgdir, va, pa, perm));
  }
}

void vm_start(void) {
  rv64_satp satp_reg = {
      .bits = {.mode = SV39, .asid = 0, .ppn = ((unsigned long)&kern_pgdir) / PGSIZE}};
  csrw_satp(satp_reg.val);
  sfence_vma;
}

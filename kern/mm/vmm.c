#include <aligns.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lib/regs.h>
#include <kern/lib/sync.h>
#include <kern/mm/pmm.h>
#include <kern/mm/vmm.h>
#include <layouts.h>
#include <lib/string.h>
#include <stddef.h>

pte_t kern_pgdir[PGSIZE / sizeof(pte_t)] __attribute__((aligned(PGSIZE)));

static void *kalloc(size_t size, size_t align) {
  UNIMPLEMENTED;
}

static void kfree(const void *ptr) {
  UNIMPLEMENTED;
}

static long pt_boot_frame_alloc(paddr_t *pa) {
  void *addr = alloc(PGSIZE, PGSIZE);
  pa->val = (unsigned long)addr;
  return KER_SUCCESS;
}

static long pt_boot_frame_free(paddr_t pa) {
  UNIMPLEMENTED;
}

static long pt_phy_frame_alloc(paddr_t *pa) {
  struct phy_frame *pf;
  catch_e(phy_frame_alloc(&pf));
  panic_e(phy_frame_ref_inc(pf));
  panic_e(frame2pa(pf, &pa->val));
  return KER_SUCCESS;
}

static long pt_phy_frame_free(paddr_t pa) {
  struct phy_frame *pf;
  catch_e(pa2frame(pa.val, &pf));
  panic_e(phy_frame_ref_dec(pf));
  return KER_SUCCESS;
}

static long (*pt_frame_alloc)(paddr_t *pa) = pt_boot_frame_alloc;

static long (*pt_frame_free)(paddr_t pa) = pt_boot_frame_free;

static long pt_walk(pte_t *pgdir, vaddr_t va, int create, pte_t **pte) {
  paddr_t pa1 = {.val = 0};
  paddr_t pa2 = {.val = 0};
  pte_t *ptr = pgdir + va.bits.vpn2;
  if (!ptr->bits.v) {
    if (!create) {
      *pte = NULL;
      return KER_SUCCESS;
    }
    catch_e(pt_frame_alloc(&pa1));
    ptr->val = 0;
    ptr->bits.v = 1;
    ptr->pp.ppn = pa1.pp.ppn;
  }

  ptr = (pte_t *)pte2pa(*ptr).val + va.bits.vpn1;
  if (!ptr->bits.v) {
    if (!create) {
      *pte = NULL;
      return KER_SUCCESS;
    }
    catch_e(pt_frame_alloc(&pa2), {
      if (pa1.val != 0) {
        panic_e(pt_frame_free(pa1));
      }
      return err;
    });
    ptr->val = 0;
    ptr->bits.v = 1;
    ptr->pp.ppn = pa2.pp.ppn;
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

struct mmio_setup {
  mmio_setup_callback_t mm_callback;
  LIST_ENTRY(mmio_setup) mm_link;
};

static LIST_HEAD(__magic, mmio_setup) vmm_mmio_setup_queue;

void vmm_register_mmio(mmio_setup_callback_t callback) {
  struct mmio_setup *cb_node = alloc(sizeof(struct mmio_setup), sizeof(struct mmio_setup));
  cb_node->mm_callback = callback;
  LIST_INSERT_HEAD(&vmm_mmio_setup_queue, cb_node, mm_link);
}

void vmm_setup_mmio(void) {
  struct mmio_setup *cb_node;
  LIST_FOREACH (cb_node, &vmm_mmio_setup_queue, mm_link) {
    panic_e(cb_invoke(cb_node->mm_callback)());
  }
}

void vmm_setup_kern(void) {
  extern uint8_t kern_text_end[];
  unsigned long freemem_base = mm_get_freemem_base();
  vaddr_t va = {.val = KERNBASE};
  paddr_t pa = {.val = KERNBASE};
  size_t text_end = align_up((size_t)kern_text_end, PGSIZE);
  size_t free_end = align_up((size_t)freemem_base, PGSIZE);

  for (; va.val < text_end; va.val += PGSIZE, pa.val += PGSIZE) {
    perm_t perm = {.bits = {.a = 1, .d = 1, .r = 1, .x = 1, .w = 1, .g = 1}};
    panic_e(pt_map(kern_pgdir, va, pa, perm));
  }

  for (; va.val < free_end; va.val += PGSIZE, pa.val += PGSIZE) {
    perm_t perm = {.bits = {.a = 1, .d = 1, .r = 1, .w = 1, .g = 1}};
    panic_e(pt_map(kern_pgdir, va, pa, perm));
  }

  alloc = kalloc;
  free = kfree;
  pt_frame_alloc = pt_phy_frame_alloc;
  pt_frame_free = pt_phy_frame_free;
}

void vmm_start(void) {
  rv64_satp satp_reg = {
      .bits = {.mode = SV39, .asid = 0, .ppn = ((unsigned long)&kern_pgdir) / PGSIZE}};
  csrw_satp(satp_reg.val);
  sfence_vma;
  info("enable virtual memory with satp: %016lx\n", satp_reg.val);
}

void vmm_flush(void) {
  sfence_vma;
}

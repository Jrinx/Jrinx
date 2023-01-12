#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lib/sync.h>
#include <kern/mm/pmm.h>
#include <kern/mm/vmm.h>
#include <layouts.h>

void vmm_test(void) {
  struct phy_frame *frame;

  info("test page mapping\n");
  assert(phy_frame_alloc(&frame) == KER_SUCCESS);
  unsigned long ref = frame->pf_ref;
  paddr_t pa;
  assert(frame2pa(frame, &pa.val) == KER_SUCCESS);
  vaddr_t va = {.val = pa.val + DEVOFFSET};
  perm_t perm = {.bits = {.a = 1, .d = 1, .r = 1, .w = 1}};
  assert(pt_map(kern_pgdir, va, pa, perm) == KER_SUCCESS);
  assert(frame->pf_ref == ref + 1);
  sfence_vma;

  info("test space sharing\n");
  unsigned long *space[] = {(unsigned long *)pa.val, (unsigned long *)va.val};

  for (size_t i = 0; i < PGSIZE / sizeof(unsigned long); i++) {
    unsigned long *src = space[i % 2];
    unsigned long *dst = space[!(i % 2)];
    src[i] = i;
    assert(dst[i] == i);
    src[i] = ~dst[i];
    assert(src[i] == dst[i]);
  }

  info("test page unmapping\n");
  assert(pt_unmap(kern_pgdir, va) == KER_SUCCESS);
  assert(frame->pf_ref == ref);
  sfence_vma;
}

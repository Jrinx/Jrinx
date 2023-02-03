#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/mm/pmm.h>
#include <kern/tests.h>

static void pmm_test(void) {
  struct phy_frame *frame1;
  struct phy_frame *frame2;
  struct phy_frame *tmp_frame;
  unsigned long addr1;
  unsigned long addr2;

  info("test allocation\n");
  assert(phy_frame_alloc(&frame1) == KER_SUCCESS);
  assert(frame2pa(frame1, &addr1) == KER_SUCCESS);
  assert(frame1->pf_ref == 0);

  assert(phy_frame_alloc(&frame2) == KER_SUCCESS);
  assert(frame2pa(frame2, &addr2) == KER_SUCCESS);
  assert(frame2->pf_ref == 0);

  assert(frame1 != frame2);
  assert(addr1 != addr2);

  info("test frame ref\n");
  assert(phy_frame_ref_dec(frame1) == -KER_MEM_ER);
  assert(phy_frame_ref_inc(frame1) == KER_SUCCESS);
  assert(frame1->pf_ref == 1);
  assert(phy_frame_ref_inc(frame1) == KER_SUCCESS);
  assert(frame1->pf_ref == 2);
  assert(phy_frame_ref_dec(frame1) == KER_SUCCESS);
  assert(frame1->pf_ref == 1);

  assert(phy_frame_ref_inc(frame2) == KER_SUCCESS);
  assert(frame2->pf_ref == 1);

  info("test memory limit exceeding\n");
  while (phy_frame_alloc(&tmp_frame) == KER_SUCCESS) {
  }

  assert(phy_frame_alloc(&tmp_frame) == -KER_MEM_ER);

  info("test frame recycling\n");
  assert(phy_frame_ref_dec(frame1) == KER_SUCCESS);
  assert(phy_frame_alloc(&tmp_frame) == KER_SUCCESS);
}

static struct kern_test pmm_testcase = {
    .kt_name = "pmm-test",
    .kt_test_func = pmm_test,
};

kern_test_def(pmm_testcase);

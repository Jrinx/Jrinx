#include <bitmap.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/mm/asid.h>
#include <kern/tests.h>
#include <lib/string.h>

static void asid_test(void) {
  info("test allocation\n");
  BITMAP_DECL(asid_allocated, asid_get_max() + 1);
  memset(asid_allocated, 0, sizeof(asid_allocated));
  unsigned asid_allocated_max = 0;
  unsigned long asid;
  for (unsigned i = 0; i <= asid_get_max(); i++) {
    assert(asid_alloc(&asid) == KER_SUCCESS);
    assert(asid >= 0 && asid <= asid_get_max());
    assert(bitmap_get_bit(asid_allocated, asid) == 0);
    bitmap_set_bit(asid_allocated, asid);
    asid_allocated_max = asid > asid_allocated_max ? asid : asid_allocated_max;
  }
  assert(asid_allocated_max == asid_get_max());

  info("test asid limit exceeding\n");
  assert(asid_alloc(&asid) == -KER_ASID_ER);

  info("test asid recycling\n");
  assert(asid_free(asid_allocated_max / 2) == KER_SUCCESS);
  assert(asid_alloc(&asid) == KER_SUCCESS);
  assert(asid == asid_allocated_max / 2);

  info("test generation\n");
  assert(asid_alloc(&asid) != KER_SUCCESS);
  uint64_t old_generation = asid_get_generation();
  asid_inc_generation();
  assert(old_generation + 1 == asid_get_generation());
  assert(asid_alloc(&asid) == KER_SUCCESS);
}

static struct kern_test asid_testcase = {
    .kt_name = "asid-test",
    .kt_test_func = asid_test,
};

kern_test_def(asid_testcase);

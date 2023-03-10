#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/mm/asid.h>
#include <kern/tests.h>
#include <lib/string.h>

static unsigned asid_allocated[0x10000U];
static unsigned asid_allocated_max = 0;

static void asid_test(void) {
  info("test allocation\n");
  assert(asid_get_max() > 0);
  unsigned long asid;
  for (unsigned i = 0; i <= asid_get_max(); i++) {
    assert(asid_alloc(&asid) == KER_SUCCESS);
    assert(asid >= 0 && asid <= asid_get_max());
    assert(asid_allocated[asid] == 0);
    asid_allocated[asid] = 1;
    asid_allocated_max = asid > asid_allocated_max ? asid : asid_allocated_max;
  }
  assert(asid_allocated_max == asid_get_max());

  info("test asid limit exceeding\n");
  assert(asid_alloc(&asid) == -KER_ASID_ER);

  info("test asid recycling\n");
  assert(asid_free(asid_allocated_max / 2) == KER_SUCCESS);
  assert(asid_alloc(&asid) == KER_SUCCESS);
  assert(asid == asid_allocated_max / 2);
}

static struct kern_test asid_testcase = {
    .kt_name = "asid-test",
    .kt_test_func = asid_test,
};

kern_test_def(asid_testcase);

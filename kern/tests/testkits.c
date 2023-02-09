#include <kern/lib/debug.h>
#include <kern/tests.h>
#include <lib/string.h>
#include <stddef.h>

void do_test(const char *name) {
  extern struct kern_test *kern_testset_begin[];
  extern struct kern_test *kern_testset_end[];
  int found = 0;
  for (struct kern_test **ptr = kern_testset_begin; ptr < kern_testset_end; ptr++) {
    struct kern_test *test = *ptr;
    if (strcmp(test->kt_name, name) == 0) {
      info("<<< %s begin\n", name);
      test->kt_test_func();
      info(">>> %s end\n", name);
      found = 1;
      break;
    }
  }
  assert(found);
}

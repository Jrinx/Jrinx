#include <kern/lib/debug.h>
#include <kern/tests.h>
#include <lib/string.h>
#include <stddef.h>

struct kern_test {
  char *kt_name;
  void (*kt_test_func)(void);
};

void pmm_test(void) __attribute__((weak));

static struct kern_test kern_testset[] = {
    {"pmm-test", pmm_test},
};

void do_test(const char *name) {
  int found = 0;
  for (size_t i = 0; i < sizeof(kern_testset) / sizeof(struct kern_test); i++) {
    struct kern_test *test = &kern_testset[i];
    if (strcmp(test->kt_name, name) == 0) {
      if (test->kt_test_func == NULL) {
        fatal("%s not linked into kernel\n");
      }
      info("<<< %s begin\n", name);
      test->kt_test_func();
      info(">>> %s end\n", name);
      found = 1;
      break;
    }
  }
  assert(found);
}

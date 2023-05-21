#ifndef _KERN_TESTS_H_
#define _KERN_TESTS_H_

#include <attr.h>

struct kern_test {
  char *kt_name;
  void (*kt_test_func)(void);
};

#define kern_test_def(name)                                                                    \
  struct kern_test *kern_##name##_def __section(".ksec.testcase." #name) __used = &name

#endif

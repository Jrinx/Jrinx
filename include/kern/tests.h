#ifndef _KERN_TESTS_H_
#define _KERN_TESTS_H_

struct kern_test {
  char *kt_name;
  void (*kt_test_func)(void);
};

#define kern_test_def(name)                                                                    \
  struct kern_test *kern_##name##_def                                                          \
      __attribute__((section(".ksec.testcase." #name), used)) = &name

#endif

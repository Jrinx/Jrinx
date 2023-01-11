#ifndef _KERN_TESTS_H_
#define _KERN_TESTS_H_

void do_test(const char *name);

typedef void (*kern_test_func_t)(void);
void pmm_test(void) __attribute__((weak));

#endif

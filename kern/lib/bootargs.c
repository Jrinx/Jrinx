#include <kern/drivers/devicetree.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/tests.h>
#include <lib/argparser.h>
#include <lib/string.h>

#define BOOTARGS_MAX_LEN 128
#define BOOTARGS_MAX_NUM 32

static char args_raw[BOOTARGS_MAX_LEN];
static const char *args_list[BOOTARGS_MAX_NUM];
static size_t args_cnt = 0;
static const char *args_test;
static int args_dt_print = 0;
static struct arg_opt args_collections[] = {
    arg_of_str('t', "test", &args_test),
    arg_of_bool('d', "dt-print", &args_dt_print),
    arg_of_end,
};

static void do_test(const char *name) {
  extern struct kern_test *kern_testset_begin[];
  extern struct kern_test *kern_testset_end[];
  for (struct kern_test **ptr = kern_testset_begin; ptr < kern_testset_end; ptr++) {
    struct kern_test *test = *ptr;
    if (strcmp(test->kt_name, name) == 0) {
      info("<<< %s begin\n", name);
      test->kt_test_func();
      info(">>> %s end\n", name);
      return;
    }
  }
  fatal("test %s not found\n", name);
}

static long args_action(void) {
  if (args_test != NULL) {
    do_test(args_test);
    halt("arg-driven test done, halt!\n");
  }
  if (args_dt_print) {
    extern struct dev_tree boot_dt;
    dt_print_tree(&boot_dt);
    halt("arg-dirven print boot device tree done, halt!\n");
  }
  return KER_SUCCESS;
}

long args_evaluate(const char *bootargs) {
  if (bootargs == NULL) {
    return KER_SUCCESS;
  }

  while (bootargs[0] && bootargs[0] == ' ') {
    bootargs++;
  }
  size_t bootargs_len = strlen(bootargs);
  while (bootargs_len > 1 && bootargs[bootargs_len - 1] == ' ') {
    bootargs_len--;
  }
  if (bootargs_len == 0) {
    return KER_SUCCESS;
  }
  if (bootargs_len + 1 >= BOOTARGS_MAX_LEN) {
    return -KER_ARG_ER;
  }

  memcpy(args_raw, bootargs, bootargs_len);
  args_raw[bootargs_len] = ' ';
  args_raw[bootargs_len + 1] = '\0';

  for (size_t i = 0, j = 0, k = 0; i < bootargs_len + 1; i++) {
    if (args_raw[i] == ' ') {
      if (k == 0) { // IDLE
        j = i;
      } else { // FETCH
        args_raw[i] = '\0';
        args_list[args_cnt++] = &args_raw[j];
        j = i;
        k = 0;
      }
    } else if (k == 0) { // IDLE
      j = i;
      k = 1;
    }
  }

  parse_ret_t ret = args_parse(args_collections, args_cnt, args_list);
  if (ret.error != ARGP_SUCCESS) {
    return -KER_ARG_ER;
  }
  return args_action();
}

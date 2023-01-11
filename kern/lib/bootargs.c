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
static struct arg_opt args_collections[] = {
    arg_of_str('t', "test", &args_test),
};

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
  return ret.error == ARGP_SUCCESS ? KER_SUCCESS : -KER_ARG_ER;
}

long args_action(void) {
  if (args_test != NULL) {
    do_test(args_test);
    halt("arg-driven test done, halt!\n");
  }
  return KER_SUCCESS;
}

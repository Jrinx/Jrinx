#include <kern/drivers/devicetree.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/mm/pmm.h>
#include <kern/multitask/partition.h>
#include <kern/tests.h>
#include <lib/argparser.h>
#include <lib/string.h>

static const char *args_test;
static int args_dt_print = 0;
int args_debug_proc_run = 0;
int args_debug_sched_max_cnt = 0;
static const char *args_partitions_conf;

static struct arg_opt args_collections[] = {
    arg_of_str('t', "test", &args_test),
    arg_of_str('p', "pa-conf", &args_partitions_conf),
    arg_of_bool('d', "dt-print", &args_dt_print),
    arg_of_bool('p', "debug-proc-run", &args_debug_proc_run),
    arg_of_int('s', "debug-sched-max-cnt", &args_debug_sched_max_cnt),
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

static long do_partitions_create(const char *conf) {
  size_t conf_len = strlen(conf);
  char *conf_raw = alloc((conf_len + 1) * sizeof(char), sizeof(char));
  strcpy(conf_raw, conf);
  size_t conf_items_cnt = 1;
  size_t conf_parts_cnt = 1;
  for (size_t i = 0; conf_raw[i] != '\0'; i++) {
    if (conf_raw[i] == ';' || conf_raw[i] == ',' || conf_raw[i] == '=') {
      conf_items_cnt++;
      if (conf_raw[i] == ';') {
        conf_parts_cnt++;
      }
      conf_raw[i] = '\0';
    }
  }
  struct part_conf *part_confs =
      alloc(conf_parts_cnt * sizeof(struct part_conf), sizeof(struct part_conf));
  size_t p = 0, q = 0;
  do {
    struct part_conf *pc = &part_confs[p++];
    pc->pa_name = NULL;
    pc->pa_prog = NULL;
    pc->pa_mem_req = 0;
    while (pc->pa_name == NULL || pc->pa_prog == NULL || pc->pa_mem_req == 0) {
      if (q > conf_len) {
        return -KER_ARG_ER;
      }
      if (strcmp(&conf_raw[q], "name") == 0) {
        pc->pa_name = &conf_raw[q + sizeof("name")];
      } else if (strcmp(&conf_raw[q], "prog") == 0) {
        pc->pa_prog = &conf_raw[q + sizeof("prog")];
      } else if (strcmp(&conf_raw[q], "memory") == 0) {
        pc->pa_mem_req = atoi(&conf_raw[q + sizeof("memory")]);
        if (pc->pa_mem_req == 0) {
          return -KER_ARG_ER;
        }
      } else {
        return -KER_ARG_ER;
      }
      for (size_t j = 0; j < 2; j++) {
        while (conf_raw[q++] != '\0') {
        }
      }
    }
  } while (p < conf_parts_cnt);

  for (size_t i = 0; i < conf_parts_cnt; i++) {
    catch_e(part_create(&part_confs[i]));
  }
  return KER_SUCCESS;
}

long args_action(void) {
  if (args_test != NULL) {
    do_test(args_test);
    halt("arg-driven test done, halt!\n");
  }
  if (args_dt_print) {
    extern struct dev_tree boot_dt;
    dt_print_tree(&boot_dt);
    halt("arg-dirven print boot device tree done, halt!\n");
  }
  if (args_partitions_conf != NULL) {
    catch_e(do_partitions_create(args_partitions_conf));
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
  char *args_raw = alloc(sizeof(char) * (bootargs_len + 1), sizeof(char));
  size_t args_cnt = 0;

  memcpy(args_raw, bootargs, bootargs_len);
  args_raw[bootargs_len] = ' ';
  args_raw[bootargs_len + 1] = '\0';

  for (size_t i = 0, k = 0; i < bootargs_len + 1; i++) {
    if (args_raw[i] == ' ') {
      if (k != 0) {
        args_raw[i] = '\0';
        args_cnt++;
        k = 0;
      }
    } else if (k == 0) {
      k = 1;
    }
  }
  const char **args_list = alloc(sizeof(char *) * args_cnt, sizeof(char *));
  size_t p = 0, q = 0;
  do {
    args_list[p++] = &args_raw[q];
    while (args_raw[q++] != '\0') {
    }
  } while (p < args_cnt);

  parse_ret_t ret = args_parse(args_collections, args_cnt, args_list);
  if (ret.error != ARGP_SUCCESS) {
    return -KER_ARG_ER;
  }
  return KER_SUCCESS;
}

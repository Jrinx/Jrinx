#include <kern/drivers/devicetree.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/mm/kalloc.h>
#include <kern/multitask/partition.h>
#include <kern/multitask/sched.h>
#include <kern/tests.h>
#include <lib/argparser.h>
#include <lib/string.h>

static const char *args_test;
static int args_debug_dt = 0;
int args_debug_kalloc_used = 0;
int args_debug_as_switch = 0;
static const char *args_partitions_conf;
static const char *args_scheduler_conf;

static struct arg_opt args_collections[] = {
    arg_of_str('t', "test", &args_test),
    arg_of_str('p', "pa-conf", &args_partitions_conf),
    arg_of_str('s', "sc-conf", &args_scheduler_conf),
    arg_of_bool(0, "debug-dt", &args_debug_dt),
    arg_of_bool(0, "debug-kalloc-used", &args_debug_kalloc_used),
    arg_of_bool(0, "debug-as-switch", &args_debug_as_switch),
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
  char *conf_raw = kalloc((conf_len + 1) * sizeof(char));
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
  struct part_conf *part_confs = kalloc(conf_parts_cnt * sizeof(struct part_conf));
  size_t p = 0, q = 0;
  do {
    struct part_conf *pc = &part_confs[p++];
    unsigned long pc_init = 0;
    while (pc_init < 5) {
      if (q > conf_len) {
        goto error;
      }
      if (strcmp(&conf_raw[q], "name") == 0) {
        pc->pa_name = &conf_raw[q + sizeof("name")];
        pc_init++;
      } else if (strcmp(&conf_raw[q], "prog") == 0) {
        pc->pa_prog = &conf_raw[q + sizeof("prog")];
        pc_init++;
      } else if (strcmp(&conf_raw[q], "memory") == 0) {
        pc->pa_mem_req = atoi(&conf_raw[q + sizeof("memory")]);
        if (pc->pa_mem_req == 0) {
          goto error;
        }
        pc_init++;
      } else if (strcmp(&conf_raw[q], "period") == 0) {
        pc->pa_period = atoi(&conf_raw[q + sizeof("period")]);
        pc_init++;
      } else if (strcmp(&conf_raw[q], "duration") == 0) {
        pc->pa_duration = atoi(&conf_raw[q + sizeof("duration")]);
        pc_init++;
      } else {
        goto error;
      }
      for (size_t j = 0; j < 2; j++) {
        while (conf_raw[q++] != '\0') {
        }
      }
    }
  } while (p < conf_parts_cnt);

  for (size_t i = 0; i < conf_parts_cnt; i++) {
    catch_e(part_create(&part_confs[i]), { goto error; });
  }
  kfree(conf_raw);
  kfree(part_confs);
  return KER_SUCCESS;
error:
  kfree(conf_raw);
  kfree(part_confs);
  return -KER_ARG_ER;
}

static long do_sched_create(const char *conf) {
  size_t conf_len = strlen(conf);
  char *conf_raw = kalloc((conf_len + 1) * sizeof(char));
  strcpy(conf_raw, conf);
  size_t conf_items_cnt = 1;
  size_t conf_scheds_cnt = 1;
  for (size_t i = 0; conf_raw[i] != '\0'; i++) {
    if (conf_raw[i] == ';' || conf_raw[i] == ',' || conf_raw[i] == '=') {
      conf_items_cnt++;
      if (conf_raw[i] == ';') {
        conf_scheds_cnt++;
      }
      conf_raw[i] = '\0';
    }
  }
  struct sched_conf *sched_confs = kalloc(conf_scheds_cnt * sizeof(struct sched_conf));
  size_t p = 0, q = 0;
  do {
    struct sched_conf *sc = &sched_confs[p++];
    unsigned long sc_init = 0;
    while (sc_init < 3) {
      if (q > conf_len) {
        goto error;
      }
      if (strcmp(&conf_raw[q], "part") == 0) {
        sc->sc_pa_name = &conf_raw[q + sizeof("part")];
        sc_init++;
      } else if (strcmp(&conf_raw[q], "offset") == 0) {
        sc->sc_offset = atoi(&conf_raw[q + sizeof("offset")]);
        sc_init++;
      } else if (strcmp(&conf_raw[q], "duration") == 0) {
        sc->sc_duration = atoi(&conf_raw[q + sizeof("duration")]);
        sc_init++;
      } else {
        goto error;
      }
      for (size_t j = 0; j < 2; j++) {
        while (conf_raw[q++] != '\0') {
        }
      }
    }
  } while (p < conf_scheds_cnt);
  for (size_t i = 0; i < conf_scheds_cnt; i++) {
    catch_e(sched_module_add(&sched_confs[i]), { goto error; });
  }
  kfree(conf_raw);
  kfree(sched_confs);
  return KER_SUCCESS;
error:
  kfree(conf_raw);
  kfree(sched_confs);
  return -KER_ARG_ER;
}

long args_action(void) {
  if (args_test != NULL) {
    do_test(args_test);
    halt("arg-driven test done, halt!\n");
  }
  if (args_debug_dt) {
    extern struct dev_tree boot_dt;
    dt_print_tree(&boot_dt);
    halt("arg-dirven print boot device tree done, halt!\n");
  }
  if (args_partitions_conf != NULL) {
    catch_e(do_partitions_create(args_partitions_conf));
  }
  if (args_scheduler_conf != NULL) {
    catch_e(do_sched_create(args_scheduler_conf));
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
  char *args_raw = kalloc(sizeof(char) * (bootargs_len + 1));
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
  const char **args_list = kalloc(sizeof(char *) * args_cnt);
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

#include <kern/chan/channel.h>
#include <kern/chan/queuing.h>
#include <kern/drivers/devicetree.h>
#include <kern/lib/boottime.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/mm/kalloc.h>
#include <kern/multitask/partition.h>
#include <kern/multitask/sched.h>
#include <kern/tests.h>
#include <kern/traps/timer.h>
#include <lib/argparser.h>
#include <lib/string.h>

static const char *args_test;
static int args_debug_dt = 0;
int args_debug_kalloc_used = 0;
int args_debug_as_switch = 0;
int args_debug_tick_interval = 0;
int args_debug_tick_num = 0;
static const char *args_partitions_conf;
static const char *args_scheduler_conf;
static const char *args_sampling_ports_conf;
static const char *args_queuing_ports_conf;
static const char *args_channel_conf;

static struct arg_opt args_collections[] = {
    arg_of_str('t', "test", &args_test),
    arg_of_str('p', "pa-conf", &args_partitions_conf),
    arg_of_str('h', "sc-conf", &args_scheduler_conf),
    arg_of_str('s', "sp-conf", &args_sampling_ports_conf),
    arg_of_str('q', "qp-conf", &args_queuing_ports_conf),
    arg_of_str('c', "ch-conf", &args_channel_conf),
    arg_of_bool(0, "debug-dt", &args_debug_dt),
    arg_of_bool(0, "debug-kalloc-used", &args_debug_kalloc_used),
    arg_of_bool(0, "debug-as-switch", &args_debug_as_switch),
    arg_of_int(0, "debug-tick-interval", &args_debug_tick_interval),
    arg_of_int(0, "debug-tick-num", &args_debug_tick_num),
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

// TODO: REFACTOER NEEDED
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

long do_sampling_ports_register(const char *conf) {
  // TODO
  UNIMPLEMENTED;
}

long do_queuing_ports_register(const char *conf) {
  size_t conf_len = strlen(conf);
  char *conf_raw = kalloc((conf_len + 1) * sizeof(char));
  strcpy(conf_raw, conf);
  size_t conf_items_cnt = 1;
  size_t conf_ports_cnt = 1;
  for (size_t i = 0; conf_raw[i] != '\0'; i++) {
    if (conf_raw[i] == ';' || conf_raw[i] == ',' || conf_raw[i] == '=') {
      conf_items_cnt++;
      if (conf_raw[i] == ';') {
        conf_ports_cnt++;
      }
      conf_raw[i] = '\0';
    }
  }
  struct queuing_port_conf *qport_confs =
      kalloc(conf_ports_cnt * sizeof(struct queuing_port_conf));
  size_t p = 0, q = 0;
  do {
    struct queuing_port_conf *pc = &qport_confs[p++];
    unsigned long pc_init = 0;
    while (pc_init < 5) {
      if (q > conf_len) {
        goto error;
      }
      if (strcmp(&conf_raw[q], "name") == 0) {
        pc->qpc_name = &conf_raw[q + sizeof("name")];
        pc_init++;
      } else if (strcmp(&conf_raw[q], "part") == 0) {
        pc->qpc_pa_name = &conf_raw[q + sizeof("part")];
        pc_init++;
      } else if (strcmp(&conf_raw[q], "direction") == 0) {
        if (strcmp(&conf_raw[q + sizeof("direction")], "src") == 0) {
          pc->qpc_dir = SOURCE;
        } else if (strcmp(&conf_raw[q + sizeof("direction")], "dst") == 0) {
          pc->qpc_dir = DESTINATION;
        } else {
          goto error;
        }
        pc_init++;
      } else if (strcmp(&conf_raw[q], "max-msg-size") == 0) {
        pc->qpc_max_msg_size = atoi(&conf_raw[q + sizeof("max-msg-size")]);
        pc_init++;
      } else if (strcmp(&conf_raw[q], "max-nb-msg") == 0) {
        pc->qpc_max_nb_msg = atoi(&conf_raw[q + sizeof("max-nb-msg")]);
        pc_init++;
      } else {
        goto error;
      }
      for (size_t j = 0; j < 2; j++) {
        while (conf_raw[q++] != '\0') {
        }
      }
    }
  } while (p < conf_ports_cnt);
  for (size_t i = 0; i < conf_ports_cnt; i++) {
    queuing_port_register(&qport_confs[i]);
  }
  kfree(conf_raw);
  kfree(qport_confs);
  return KER_SUCCESS;
error:
  kfree(conf_raw);
  kfree(qport_confs);
  return -KER_ARG_ER;
}

long do_channel_create(const char *conf) {
  size_t conf_len = strlen(conf);
  char *conf_raw = kalloc((conf_len + 1) * sizeof(char));
  strcpy(conf_raw, conf);
  size_t conf_items_cnt = 1;
  size_t conf_channels_cnt = 1;
  for (size_t i = 0; conf_raw[i] != '\0'; i++) {
    if (conf_raw[i] == ';' || conf_raw[i] == ',' || conf_raw[i] == '=') {
      conf_items_cnt++;
      if (conf_raw[i] == ';') {
        conf_channels_cnt++;
      }
      conf_raw[i] = '\0';
    }
  }
  struct channel_conf *channel_confs = kalloc(conf_channels_cnt * sizeof(struct channel_conf));
  size_t p = 0, q = 0;
  do {
    struct channel_conf *cc = &channel_confs[p++];
    unsigned long cc_init = 0;
    while (cc_init < 3) {
      if (q > conf_len) {
        goto error;
      }
      if (strcmp(&conf_raw[q], "media") == 0) {
        if (strcmp(&conf_raw[q + sizeof("media")], "memory") == 0) {
          cc->cc_media = CM_MEMORY;
        } else {
          goto error;
        }
        cc_init++;
      } else if (strcmp(&conf_raw[q], "type") == 0) {
        if (strcmp(&conf_raw[q + sizeof("type")], "queuing") == 0) {
          cc->cc_type = CT_QUEUING;
        } else if (strcmp(&conf_raw[q + sizeof("type")], "sampling") == 0) {
          cc->cc_type = CT_SAMPLING;
        } else {
          goto error;
        }
        cc_init++;
      } else if (strcmp(&conf_raw[q], "ports") == 0) {
        char *ports = &conf_raw[q + sizeof("ports")];
        size_t ports_cnt = 1;
        for (size_t i = 0; ports[i] != '\0'; i++) {
          if (ports[i] == '&') {
            ports[i] = '\0';
            ports_cnt++;
          }
        }
        cc->cc_port_names = kalloc((ports_cnt + 1) * sizeof(char *));
        memset(cc->cc_port_names, 0, (ports_cnt + 1) * sizeof(char *));
        for (size_t i = 0, j = 0; i < ports_cnt; i++) {
          cc->cc_port_names[i] = &ports[j];
          while (ports[j++] != '\0') {
          }
        }
        cc_init++;
      } else {
        goto error;
      }
      for (size_t j = 0; j < 2; j++) {
        while (conf_raw[q++] != '\0') {
        }
      }
    }
  } while (p < conf_channels_cnt);
  for (size_t i = 0; i < conf_channels_cnt; i++) {
    catch_e(channel_create(&channel_confs[i]), { goto error; });
  }
  kfree(conf_raw);
  kfree(channel_confs);
  return KER_SUCCESS;
error:
  kfree(conf_raw);
  kfree(channel_confs);
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
    halt("arg-driven print boot device tree done, halt!\n");
  }
  if (args_debug_tick_num) {
    extern sys_time_t *debug_timer_complete_time;
    extern sys_time_t *debug_timer_claim_time;
    debug_timer_complete_time = kalloc(sizeof(sys_time_t) * args_debug_tick_num);
    memset(debug_timer_complete_time, 0, sizeof(sys_time_t) * args_debug_tick_num);
    debug_timer_claim_time = kalloc(sizeof(sys_time_t) * args_debug_tick_num);
    memset(debug_timer_claim_time, 0, sizeof(sys_time_t) * args_debug_tick_num);
    extern sys_time_t *debug_timer_claim_expected_time;
    debug_timer_claim_expected_time = kalloc(sizeof(sys_time_t) * args_debug_tick_num);
    sys_time_t now = boottime_get_now();
    const sys_time_t delay =
        2 * (SYS_TIME_SECOND > args_debug_tick_interval ? SYS_TIME_SECOND
                                                        : args_debug_tick_interval);
    for (size_t i = 0; i < args_debug_tick_num; i++) {
      debug_timer_claim_expected_time[i] = delay + now + i * args_debug_tick_interval;
      time_event_alloc(NULL, delay + now + i * args_debug_tick_interval, TE_SIMPLE);
    }
    info("arg-driven time event allocation done, first time event: %ld us\n", now + delay);
  }
  if (args_partitions_conf != NULL) {
    catch_e(do_partitions_create(args_partitions_conf));
  }
  if (args_scheduler_conf != NULL) {
    catch_e(do_sched_create(args_scheduler_conf));
  }
  if (args_sampling_ports_conf != NULL) {
    catch_e(do_sampling_ports_register(args_sampling_ports_conf));
  }
  if (args_queuing_ports_conf != NULL) {
    catch_e(do_queuing_ports_register(args_queuing_ports_conf));
  }
  if (args_channel_conf != NULL) {
    catch_e(do_channel_create(args_channel_conf));
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

#include <kern/drivers/device.h>
#include <kern/drivers/serialport.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <lib/string.h>
#include <stddef.h>

static const char *chosen_bootargs;
static const char *chosen_stdout;
static const char *chosen_stdin;

static long chosen_probe(const struct dev_node *node) {
  if (strcmp(node->nd_name, "chosen") != 0) {
    return KER_SUCCESS;
  }

  info("%s probed, props listed:\n", node->nd_name);

  struct dev_node_prop *prop;
  prop = dt_node_prop_extract(node, "bootargs");
  if (prop != NULL) {
    chosen_bootargs = (char *)prop->pr_values;
    info("\tbootargs: %s\n", chosen_bootargs);
  }

  prop = dt_node_prop_extract(node, "stdout-path");
  if (prop != NULL) {
    chosen_stdout = (char *)prop->pr_values;
    info("\tstdout-path: %s\n", chosen_stdout);
  }

  prop = dt_node_prop_extract(node, "stdin-path");
  if (prop != NULL) {
    chosen_stdin = (char *)prop->pr_values;
    info("\tstdin-path: %s\n", chosen_stdin);
  }

  return KER_SUCCESS;
}

struct device chosen_device = {
    .d_probe = chosen_probe,
    .d_probe_pri = LOWEST,
};

const char *chosen_get_bootargs(void) {
  return chosen_bootargs;
}

const char *chosen_get_stdout_dev_name(void) {
  if (chosen_stdout == NULL) {
    return NULL;
  }
  size_t i;
  size_t j;
  for (i = 0, j = 0; chosen_stdout[i]; i++) {
    if (chosen_stdout[i] == '/' && chosen_stdout[i + 1]) {
      j = i;
    }
  }
  return &chosen_stdout[j + 1];
}

const char *chosen_get_stdin_dev_name(void) {
  if (chosen_stdin == NULL) {
    return NULL;
  }
  size_t i;
  size_t j;
  for (i = 0, j = 0; chosen_stdin[i]; i++) {
    if (chosen_stdin[i] == '/' && chosen_stdin[i + 1]) {
      j = i;
    }
  }
  return &chosen_stdin[j + 1];
}

long chosen_select_dev(void) {
  const char *stdout_dev = chosen_get_stdout_dev_name();
  const char *stdin_dev = chosen_get_stdin_dev_name();
  if (stdin_dev == NULL) {
    stdin_dev = stdout_dev;
  }
  if (stdout_dev != NULL) {
    if (!serial_select_out_dev(stdout_dev)) {
      return -KER_SRL_ER;
    }
  }
  if (stdin_dev != NULL) {
    if (!serial_select_in_dev(stdin_dev)) {
      return -KER_SRL_ER;
    }
  }
  return KER_SUCCESS;
}
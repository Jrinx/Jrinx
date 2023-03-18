#include <kern/drivers/aliases.h>
#include <kern/drivers/device.h>
#include <kern/drivers/serialport.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <lib/string.h>
#include <stddef.h>

static const char *chosen_bootargs;
static const char *chosen_stdout;
static const char *chosen_stdin;

static int chosen_pred(const struct dev_node *node) {
  return strcmp(node->nd_name, "chosen") == 0;
}

static long chosen_probe(const struct dev_node *node) {
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

static struct device chosen_device = {
    .d_pred = chosen_pred,
    .d_probe = chosen_probe,
};

device_init(chosen_device, lowest);

const char *chosen_get_bootargs(void) {
  return chosen_bootargs;
}

static const char *chosen_get_dev_name(const char *path) {
  if (path == NULL) {
    return NULL;
  }
  const char *realname = aliases_get(path);
  if (realname != NULL) {
    path = realname;
  }
  size_t i;
  size_t j;
  for (i = 0, j = 0; path[i]; i++) {
    if (path[i] == '/' && path[i + 1]) {
      j = i;
    }
  }
  return &path[j + 1];
}

const char *chosen_get_stdout_dev_name(void) {
  return chosen_get_dev_name(chosen_stdout);
}

const char *chosen_get_stdin_dev_name(void) {
  return chosen_get_dev_name(chosen_stdin);
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

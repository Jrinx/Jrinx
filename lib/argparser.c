#include <lib/argparser.h>
#include <lib/string.h>
#include <stddef.h>

static parse_ret_t args_assign(struct arg_opt *opt, const char *nxt, size_t pos,
                               size_t *pos_push) {
  parse_ret_t ret = {.error = ARGP_SUCCESS, .value = 0};
  switch (opt->arg_type) {
  case ARG_BOOLEAN:
    *((int *)opt->arg_target) = 1;
    *pos_push = 0;
    break;
  case ARG_INTEGER:
    if (nxt == NULL) {
      ret.error = ARGP_MISSING_VAL;
      ret.value = pos;
    } else {
      int num = atoi(nxt);
      *((int *)opt->arg_target) = num;
    }
    *pos_push = 1;
    break;
  case ARG_STRING:
    if (nxt == NULL) {
      ret.error = ARGP_MISSING_VAL;
      ret.value = pos;
    } else {
      *((const char **)opt->arg_target) = nxt;
    }
    *pos_push = 1;
    break;
  default:
    ret.error = ARGP_PANIC;
    ret.value = pos;
  }
  return ret;
}

static struct arg_opt *args_parse_short_arg_name(struct arg_opt *arg_opts, char name) {
  for (size_t i = 0; arg_opts[i].arg_type != ARG_END; i++) {
    if (arg_opts[i].arg_short_name == name) {
      return &arg_opts[i];
    }
  }
  return NULL;
}

static parse_ret_t args_parse_short_arg(struct arg_opt *arg_opts, size_t argc,
                                        const char **argv, size_t pos, size_t *pos_push) {
  parse_ret_t ret = {.error = ARGP_SUCCESS, .value = 0};
  const char *arg = argv[pos];
  struct arg_opt *opt = args_parse_short_arg_name(arg_opts, arg[1]);
  if (opt == NULL) {
    ret.error = ARGP_UNKNOWN_ARG;
    ret.value = pos;
    return ret;
  }
  return args_assign(opt, pos + 1 < argc ? argv[pos + 1] : NULL, pos, pos_push);
}

static struct arg_opt *args_parse_long_arg_name(struct arg_opt *arg_opts, const char *name) {
  for (size_t i = 0; arg_opts[i].arg_type != ARG_END; i++) {
    if (strcmp(arg_opts[i].arg_long_name, name) == 0) {
      return &arg_opts[i];
    }
  }
  return NULL;
}

static parse_ret_t args_parse_long_arg(struct arg_opt *arg_opts, size_t argc, const char **argv,
                                       size_t pos, size_t *pos_push) {
  parse_ret_t ret = {.error = ARGP_SUCCESS, .value = 0};
  const char *arg = argv[pos];
  struct arg_opt *opt = args_parse_long_arg_name(arg_opts, &arg[2]);
  if (opt == NULL) {
    ret.error = ARGP_UNKNOWN_ARG;
    ret.value = pos;
    return ret;
  }
  return args_assign(opt, pos + 1 < argc ? argv[pos + 1] : NULL, pos, pos_push);
}

parse_ret_t args_parse(struct arg_opt *arg_opts, size_t argc, const char **argv) {
  parse_ret_t ret = {.error = ARGP_SUCCESS, .value = 0};
  for (size_t i = 0; i < argc; i++) {
    const char *arg = argv[i];
    if (arg[0] != '-') {
      ret.error = ARGP_UNKNOWN_ARG;
      ret.value = i;
      return ret;
    }
    size_t pos_push;
    if (arg[0] == '-' && arg[1] != '-' && arg[2] == '\0') {
      ret = args_parse_short_arg(arg_opts, argc, argv, i, &pos_push);
    } else if (arg[0] == '-' && arg[1] == '-' && arg[2] != '\0') {
      ret = args_parse_long_arg(arg_opts, argc, argv, i, &pos_push);
    }
    if (ret.error != ARGP_SUCCESS) {
      return ret;
    }
    i += pos_push;
  }
  return ret;
}

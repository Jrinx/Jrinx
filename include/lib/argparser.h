#ifndef _LIB_ARGPARSER_H_
#define _LIB_ARGPARSER_H_

#include <stddef.h>

enum arg_opt_t {
  ARG_END = 0,
  ARG_BOOLEAN,
  ARG_INTEGER,
  ARG_STRING,
};

enum arg_error_t {
  ARGP_SUCCESS = 0,
  ARGP_UNKNOWN_ARG,
  ARGP_MISSING_VAL,
  ARGP_PANIC,
};

struct arg_opt {
  enum arg_opt_t arg_type;
  char arg_short_name;
  char *arg_long_name;
  char *arg_help;
  void *arg_target;
};

typedef struct {
  enum arg_error_t error;
  long value;
} parse_ret_t;

parse_ret_t args_parse(struct arg_opt *arg_opts, size_t argc, const char **argv);

#define _arg_eles(s, l, t) .arg_short_name = s, .arg_long_name = l, .arg_target = t
#define arg_of_end                                                                             \
  { .arg_type = ARG_END, _arg_eles(0, NULL, NULL) }
#define arg_of_bool(s, l, t)                                                                   \
  { .arg_type = ARG_BOOLEAN, _arg_eles(s, l, t) }
#define arg_of_int(s, l, t)                                                                    \
  { .arg_type = ARG_INTEGER, _arg_eles(s, l, t) }
#define arg_of_str(s, l, t)                                                                    \
  { .arg_type = ARG_STRING, _arg_eles(s, l, t) }

#endif

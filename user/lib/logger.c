#include <lib/printfmt.h>
#include <lib/string.h>
#include <user/lib/debug.h>
#include <user/sys/services.h>
#include <user/sys/syscalls.h>

#define PRINTU_BUF_SIZE 512

static PARTITION_ID_TYPE part_id;

void logger_init(PARTITION_ID_TYPE partition_id) {
  part_id = partition_id;
}

struct printu_ctx {
  char buf[PRINTU_BUF_SIZE];
  size_t len;
};

static void printu_flush(struct printu_ctx *ctx) {
  if (ctx->len == 0) {
    return;
  }
  sys_cons_write_buf(ctx->buf, ctx->len);
  ctx->len = 0;
}

static void printu_output(void *data, const char *buf, size_t len) {
  struct printu_ctx *ctx = data;
  while (ctx->len + len > PRINTU_BUF_SIZE) {
    size_t n = PRINTU_BUF_SIZE - ctx->len;
    memcpy(ctx->buf + ctx->len, buf, n);
    buf += n;
    len -= n;
    ctx->len = PRINTU_BUF_SIZE;
    printu_flush(ctx);
  }
  memcpy(ctx->buf + ctx->len, buf, len);
  ctx->len += len;
}

static void vprintu(const char *restrict fmt, va_list ap) {
  struct printu_ctx ctx = {.len = 0};
  cb_decl(fmt_callback_t, printu_cb, printu_output, &ctx);
  vprintfmt(printu_cb, fmt, ap);
  printu_flush(&ctx);
}

void printu(const char *restrict fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vprintu(fmt, ap);
  va_end(ap);
}

void infou(const char *restrict file, unsigned long lineno, const char *restrict func,
           const char *restrict fmt, ...) {
  SYSTEM_TIME_TYPE time;
  PROCESS_ID_TYPE my_id;
  PROCESS_INDEX_TYPE my_index;
  check_e(GET_TIME(&time, &ret));
  check_e(GET_MY_ID(&my_id, &ret));
  RETURN_CODE_TYPE ret;
  GET_MY_INDEX(&my_index, &ret);
  if (ret == NO_ERROR) {
    printu("[ %pT part#%lu proc#%lu(%lu) ] %s:%lu <%s> ", &time, part_id, my_id, my_index, file,
           lineno, func);
  } else {
    printu("[ %pT part#%lu proc#%lu ] %s:%lu <%s> ", &time, part_id, my_id, file, lineno, func);
  }
  va_list ap;
  va_start(ap, fmt);
  vprintu(fmt, ap);
  va_end(ap);
}

void fatalu(const char *restrict file, unsigned long lineno, const char *restrict func,
            const char *restrict fmt, ...) {
  PROCESS_ID_TYPE my_id;
  PROCESS_INDEX_TYPE my_index;
  check_e(GET_MY_ID(&my_id, &ret));
  RETURN_CODE_TYPE ret;
  GET_MY_INDEX(&my_index, &ret);
  if (ret == NO_ERROR) {
    printu("[ part#%lu proc#%lu index=%lu ] fatal at %s:%lu <%s> ", part_id, my_id, my_index,
           file, lineno, func);
  } else {
    printu("[ part#%lu proc#%lu ] fatal at %s:%lu <%s> ", part_id, my_id, file, lineno, func);
  }
  va_list ap;
  va_start(ap, fmt);
  vprintu(fmt, ap);
  va_end(ap);
  sys_halt();
}

void haltu(const char *restrict file, unsigned long lineno, const char *restrict func,
           const char *restrict fmt, ...) {
  PROCESS_ID_TYPE my_id;
  PROCESS_INDEX_TYPE my_index;
  check_e(GET_MY_ID(&my_id, &ret));
  RETURN_CODE_TYPE ret;
  GET_MY_INDEX(&my_index, &ret);
  if (ret == NO_ERROR) {
    printu("[ part#%lu proc#%lu index=%lu ] halt at %s:%lu <%s> ", part_id, my_id, my_index,
           file, lineno, func);
  } else {
    printu("[ part#%lu proc#%lu ] halt at %s:%lu <%s> ", part_id, my_id, file, lineno, func);
  }
  va_list ap;
  va_start(ap, fmt);
  vprintu(fmt, ap);
  va_end(ap);
  sys_halt();
}

#include <lib/printfmt.h>
#include <lib/string.h>
#include <user/sys/syscalls.h>

#define PRINTU_BUF_SIZE 512

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

void printu(const char *restrict fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  struct printu_ctx ctx = {.len = 0};
  cb_decl(fmt_callback_t, printu_cb, printu_output, &ctx);
  vprintfmt(printu_cb, fmt, ap);
  printu_flush(&ctx);
  va_end(ap);
}

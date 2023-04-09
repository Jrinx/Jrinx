#include <errno.h>
#include <kern/drivers/serialport.h>
#include <kern/lib/debug.h>
#include <kern/multitask/sched.h>
#include <kern/traps/traps.h>
#include <sysno.h>

void do_cons_write_char(int ch) {
  serial_blocked_putc(ch);
}

int do_cons_read_char(void) {
  return serial_blocked_getc();
}

void do_cons_write_buf(const char *buf, size_t len) {
  for (size_t i = 0; i < len; i++) {
    serial_blocked_putc(buf[i]);
  }
}

void do_syscall(struct context *context) {
  // TODO: check pointer and enum from user space
  context->ctx_sepc += sizeof(uint32_t);
  unsigned long sysno = context->ctx_regs.names.a7;
  ret_code_t ret = NO_ERROR;
  switch (sysno) {
  case SYS_CONS_WRITE_CHAR:
    do_cons_write_char(context->ctx_regs.names.a0);
    break;
  case SYS_CONS_READ_CHAR:
    ret = do_cons_read_char();
    break;
  case SYS_CONS_WRITE_BUF:
    do_cons_write_buf((char *)context->ctx_regs.names.a0, context->ctx_regs.names.a1);
    break;
  case SYS_YIELD:
    sched();
    break;
  default:
    ret = INVALID_SYSNO;
    break;
  }
  context->ctx_regs.names.a0 = ret;
}

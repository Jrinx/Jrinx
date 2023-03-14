#include <errno.h>
#include <kern/drivers/serialport.h>
#include <kern/lib/debug.h>
#include <kern/multitask/sched.h>
#include <kern/traps/traps.h>
#include <sysno.h>

long do_write_cons(int ch) {
  serial_blocked_putc(ch);
  return E_NOERR;
}

long do_read_cons(void) {
  return serial_blocked_getc();
}

void do_syscall(struct context *context) {
  context->ctx_sepc += sizeof(long);
  unsigned long sysno = context->ctx_regs.names.a7;
  long ret;
  switch (sysno) {
  case SYS_WRITE_CONS:
    ret = do_write_cons(context->ctx_regs.names.a0);
    break;
  case SYS_READ_CONS:
    ret = do_read_cons();
    break;
  case SYS_YIELD:
    sched();
    break;
  default:
    ret = -E_SYSNO;
    break;
  }
  context->ctx_regs.names.a0 = ret;
}

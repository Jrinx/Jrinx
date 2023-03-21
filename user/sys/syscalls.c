#include <stddef.h>
#include <sysno.h>

static inline unsigned long syscall(unsigned long sysno, unsigned long arg0, unsigned long arg1,
                                    unsigned long arg2, unsigned long arg3, unsigned long arg4,
                                    unsigned long arg5, unsigned long arg6) {
  register long a0 asm("a0") = arg0;
  register long a1 asm("a1") = arg1;
  register long a2 asm("a2") = arg2;
  register long a3 asm("a3") = arg3;
  register long a4 asm("a4") = arg4;
  register long a5 asm("a5") = arg5;
  register long a6 asm("a6") = arg6;
  register long a7 asm("a7") = sysno;
  asm volatile("ecall"
               : "+r"(a0), "+r"(a1)
               : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7)
               : "memory");
  return a0;
}

void sys_cons_write_char(int ch) {
  syscall(SYS_CONS_WRITE_CHAR, ch, 0, 0, 0, 0, 0, 0);
}

int sys_cons_read_char(void) {
  return syscall(SYS_CONS_READ_CHAR, 0, 0, 0, 0, 0, 0, 0);
}

void sys_cons_write_buf(const char *buf, size_t len) {
  syscall(SYS_CONS_WRITE_BUF, (unsigned long)buf, len, 0, 0, 0, 0, 0);
}

void sys_yield(void) {
  syscall(SYS_YIELD, 0, 0, 0, 0, 0, 0, 0);
}

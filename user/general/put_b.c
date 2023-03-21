#include <user/lib/logger.h>
#include <user/sys/syscalls.h>

const char ch = 'B';

int main(void) {
  printu("put '%c' to console\n", ch);
  sys_yield();
  while (1) {
    sys_cons_write_char(ch);
    sys_yield();
  }
  return 0;
}

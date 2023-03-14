#include <user/lib/syscalls.h>

int main(void) {
  while (1) {
    sys_write_cons('B');
    sys_yield();
  }
  return 0;
}

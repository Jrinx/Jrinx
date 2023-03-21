#include <sysno.h>

extern int main(void);

__attribute__((noreturn)) void _osmain(void) {
  int ret __attribute__((unused)) = main();
  while (1) {
  }
}

#include <kern/lib/printk.h>
#include <types.h>

void kernel_init(u_int hartid) {
  printk("[hart %d] Hello Jrinx!\n", hartid);
  while (1) {
  }
}

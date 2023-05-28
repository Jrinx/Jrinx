#include <asm.h>
#include <layouts.h>

.text
.section .boot
EXPORT(_start)
  ld t0, cpus_stacktop
  beq t0, zero, init_stk
  sll t1, a0, 3
  add t1, t0, t1
  ld sp, (t1)
  j init_kern_call
init_stk:
  ld sp, kern_master_stacktop
init_kern_call:
  call kernel_init

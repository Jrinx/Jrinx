#include <asm/asm.h>
#include <layouts.h>

.data

.align 12
.section .ksec.stack
.globl kstack
kstack:
.space KSTKSIZE

.text
.section .boot
EXPORT(_start)
.extern cpu_master_init
  lw t0, cpu_master_init
  beq t0, zero, init_stk
  mv sp, a1
  j init_kern_call
init_stk:
  li sp, KSTKTOP
init_kern_call:
  call kernel_init

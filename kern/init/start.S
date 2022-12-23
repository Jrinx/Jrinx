#include <asm/asm.h>
#include <layouts.h>

.data

.align 12
.section .ksec.stack
.globl kstack
kstack:
.space KSTKSIZE * CONFIG_NR_CORES

.text
.section .boot
EXPORT(_start)
  add t0, a0, 1
  slli t0, t0, (PGSHIFT + KSTKSHIFT)
  la sp, kstack
  add sp, sp, t0
  call kernel_init

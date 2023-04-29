#include <asm/asm.h>
#include <kern/traps/traps.h>

.macro SAVE_REGS
  sd x0, PT_REG0(a0)
  sd x1, PT_REG1(a0)
  sd x2, PT_REG2(a0)
  sd x3, PT_REG3(a0)
  sd x4, PT_REG4(a0)
  sd x5, PT_REG5(a0)
  sd x6, PT_REG6(a0)
  sd x7, PT_REG7(a0)
  sd x8, PT_REG8(a0)
  sd x9, PT_REG9(a0)
  sd x11, PT_REG11(a0)
  sd x12, PT_REG12(a0)
  sd x13, PT_REG13(a0)
  sd x14, PT_REG14(a0)
  sd x15, PT_REG15(a0)
  sd x16, PT_REG16(a0)
  sd x17, PT_REG17(a0)
  sd x18, PT_REG18(a0)
  sd x19, PT_REG19(a0)
  sd x20, PT_REG20(a0)
  sd x21, PT_REG21(a0)
  sd x22, PT_REG22(a0)
  sd x23, PT_REG23(a0)
  sd x24, PT_REG24(a0)
  sd x25, PT_REG25(a0)
  sd x26, PT_REG26(a0)
  sd x27, PT_REG27(a0)
  sd x28, PT_REG28(a0)
  sd x29, PT_REG29(a0)
  sd x30, PT_REG30(a0)
  sd x31, PT_REG31(a0)

  fsd f0, PT_FREG0(a0)
  fsd f1, PT_FREG1(a0)
  fsd f2, PT_FREG2(a0)
  fsd f3, PT_FREG3(a0)
  fsd f4, PT_FREG4(a0)
  fsd f5, PT_FREG5(a0)
  fsd f6, PT_FREG6(a0)
  fsd f7, PT_FREG7(a0)
  fsd f8, PT_FREG8(a0)
  fsd f9, PT_FREG9(a0)
  fsd f10, PT_FREG10(a0)
  fsd f11, PT_FREG11(a0)
  fsd f12, PT_FREG12(a0)
  fsd f13, PT_FREG13(a0)
  fsd f14, PT_FREG14(a0)
  fsd f15, PT_FREG15(a0)
  fsd f16, PT_FREG16(a0)
  fsd f17, PT_FREG17(a0)
  fsd f18, PT_FREG18(a0)
  fsd f19, PT_FREG19(a0)
  fsd f20, PT_FREG20(a0)
  fsd f21, PT_FREG21(a0)
  fsd f22, PT_FREG22(a0)
  fsd f23, PT_FREG23(a0)
  fsd f24, PT_FREG24(a0)
  fsd f25, PT_FREG25(a0)
  fsd f26, PT_FREG26(a0)
  fsd f27, PT_FREG27(a0)
  fsd f28, PT_FREG28(a0)
  fsd f29, PT_FREG29(a0)
  fsd f30, PT_FREG30(a0)
  fsd f31, PT_FREG31(a0)
.endm

.macro RESTORE_REGS
  ld x1, PT_REG1(a0)
  ld x2, PT_REG2(a0)
  ld x3, PT_REG3(a0)
  ld x4, PT_REG4(a0)
  ld x5, PT_REG5(a0)
  ld x6, PT_REG6(a0)
  ld x7, PT_REG7(a0)
  ld x8, PT_REG8(a0)
  ld x9, PT_REG9(a0)
  ld x11, PT_REG11(a0)
  ld x12, PT_REG12(a0)
  ld x13, PT_REG13(a0)
  ld x14, PT_REG14(a0)
  ld x15, PT_REG15(a0)
  ld x16, PT_REG16(a0)
  ld x17, PT_REG17(a0)
  ld x18, PT_REG18(a0)
  ld x19, PT_REG19(a0)
  ld x20, PT_REG20(a0)
  ld x21, PT_REG21(a0)
  ld x22, PT_REG22(a0)
  ld x23, PT_REG23(a0)
  ld x24, PT_REG24(a0)
  ld x25, PT_REG25(a0)
  ld x26, PT_REG26(a0)
  ld x27, PT_REG27(a0)
  ld x28, PT_REG28(a0)
  ld x29, PT_REG29(a0)
  ld x30, PT_REG30(a0)
  ld x31, PT_REG31(a0)

  fld f0, PT_FREG0(a0)
  fld f1, PT_FREG1(a0)
  fld f2, PT_FREG2(a0)
  fld f3, PT_FREG3(a0)
  fld f4, PT_FREG4(a0)
  fld f5, PT_FREG5(a0)
  fld f6, PT_FREG6(a0)
  fld f7, PT_FREG7(a0)
  fld f8, PT_FREG8(a0)
  fld f9, PT_FREG9(a0)
  fld f10, PT_FREG10(a0)
  fld f11, PT_FREG11(a0)
  fld f12, PT_FREG12(a0)
  fld f13, PT_FREG13(a0)
  fld f14, PT_FREG14(a0)
  fld f15, PT_FREG15(a0)
  fld f16, PT_FREG16(a0)
  fld f17, PT_FREG17(a0)
  fld f18, PT_FREG18(a0)
  fld f19, PT_FREG19(a0)
  fld f20, PT_FREG20(a0)
  fld f21, PT_FREG21(a0)
  fld f22, PT_FREG22(a0)
  fld f23, PT_FREG23(a0)
  fld f24, PT_FREG24(a0)
  fld f25, PT_FREG25(a0)
  fld f26, PT_FREG26(a0)
  fld f27, PT_FREG27(a0)
  fld f28, PT_FREG28(a0)
  fld f29, PT_FREG29(a0)
  fld f30, PT_FREG30(a0)
  fld f31, PT_FREG31(a0)
.endm

EXPORT(trap_vec)
  csrrw a0, sscratch, a0

  SAVE_REGS

  csrr t0, sscratch
  sd t0, PT_REG10(a0)

  csrr t0, sepc
  sd t0, PT_SEPC(a0)
  csrr t0, stval
  sd t0, PT_STVAL(a0)
  csrr t0, sie
  sd t0, PT_SIE(a0)
  csrr t0, scause
  sd t0, PT_SCAUSE(a0)
  csrr t0, sstatus
  sd t0, PT_SSTATUS(a0)

  ld tp, PT_HARTID(a0)
  andi t0, t0, (1 << 8)  # SPP
  bnez t0, TRAP_FROM_S_MODE
TRAP_FROM_U_MODE:
  ld t0, cpus_cur_proc
  sll t1, tp, 3
  add t1, t0, t1
  ld t1, (t1)
  ld sp, (t1)
TRAP_FROM_S_MODE:
  call handle_trap


EXPORT(trap_ret_switch_as)
  csrr t0, sstatus
  andi t0, t0, ~(1 << 2)  # SIE
  csrw sstatus, t0
  srl t0, a1, 44
  li t1, 0xffff
  and t0, t0, t1  # ASID
  csrw satp, a1
  sfence.vma x0, t0

  # fall through to trap_ret

EXPORT(trap_ret)
  ld t0, PT_SSTATUS(a0)
  csrw sstatus, t0
  ld t0, PT_SIE(a0)
  csrw sie, t0
  ld t0, PT_STVAL(a0)
  csrw stval, t0
  ld t0, PT_SEPC(a0)
  csrw sepc, t0

  ld t0, PT_REG10(a0)
  csrw sscratch, t0

  RESTORE_REGS

  csrrw a0, sscratch, a0

  sret


EXPORT(_sched_proc_give_up)
  SAVE_REGS

  sd ra, PT_SEPC(a0)
  csrr t0, stval
  sd t0, PT_STVAL(a0)
  csrr t0, sie
  sd t0, PT_SIE(a0)
  csrr t0, scause
  sd t0, PT_SCAUSE(a0)
  csrr t0, sstatus
  andi t1, t0, (1 << 1) # SIE
  andi t2, t0, ~(1 << 5)
  sll t1, t1, 4
  or t2, t2, t1 # SPIE <- SIE
  ori t0, t2, (1 << 8)  # SPP
  sd t0, PT_SSTATUS(a0)
  sd tp, PT_HARTID(a0)
  mv a0, a1
  call sched_proc

#ifndef _KERN_LIB_REGS_H_
#define _KERN_LIB_REGS_H_

#ifdef __LP64__
#define RV_XLEN 8
#else
#define PV_XLEN 4
#endif

#ifndef __ASSEMBLER__

#include <stdint.h>

#define R_REG_DEF(reg)                                                                         \
  __attribute__((always_inline)) static inline unsigned long r_##reg(void) {                   \
    unsigned long val;                                                                         \
    asm volatile("mv %0, " #reg : "=r"(val));                                                  \
    return val;                                                                                \
  }

#define W_REG_DEF(reg)                                                                         \
  __attribute__((always_inline)) static inline void w_##reg(unsigned long val) {               \
    asm volatile("mv " #reg ", %0" : : "r"(val));                                              \
  }

#define R_CSR_DEF(reg)                                                                         \
  __attribute__((always_inline)) static inline unsigned long csrr_##reg(void) {                \
    unsigned long val;                                                                         \
    asm volatile("csrr %0, " #reg : "=r"(val));                                                \
    return val;                                                                                \
  }

#define W_CSR_DEF(reg)                                                                         \
  __attribute__((always_inline)) static inline void csrw_##reg(unsigned long val) {            \
    asm volatile("csrw " #reg ", %0" : : "r"(val));                                            \
  }

R_REG_DEF(tp)
W_REG_DEF(tp)

#define RISCV_U_MODE 0
#define RISCV_S_MODE 1
#define RISCV_H_MODE 2
#define RISCV_M_MODE 3

typedef union {
  uint64_t val;
  struct {
    unsigned blank0 : 1;
    unsigned sie : 1;
    unsigned blank1 : 3;
    unsigned spie : 1;
    unsigned ube : 1;
    unsigned blank2 : 1;
    unsigned spp : 1;
    unsigned vs : 2;
    unsigned blank3 : 2;
    unsigned fs : 2;
    unsigned xs : 2;
    unsigned blank4 : 1;
    unsigned sum : 1;
    unsigned mxr : 1;
    unsigned blank5 : 12;
    unsigned uxl : 2;
    unsigned blank6 : 29;
    unsigned sd : 1;
  } __attribute__((packed)) bits;
} rv64_sstatus;

R_CSR_DEF(sstatus)
W_CSR_DEF(sstatus)

enum rv64_stvec_mode_t {
  DIRECT = 0,
  VECTORED = 1,
};

typedef union {
  uint64_t val;
  struct {
    enum rv64_stvec_mode_t mode : 2;
    unsigned long base : 62;
  } bits;
} rv64_stvec;

R_CSR_DEF(stvec)
W_CSR_DEF(stvec)

typedef union {
  uint64_t val;
  struct {
    unsigned blank0 : 1;
    unsigned ssi : 1;
    unsigned blank1 : 3;
    unsigned sti : 1;
    unsigned blank2 : 3;
    unsigned sei : 1;
    unsigned blank3 : 6;
    unsigned long blank4 : 48;
  } __attribute__((packed)) bits;
} rv64_si;

R_CSR_DEF(sip)
W_CSR_DEF(sip)
R_CSR_DEF(sie)
W_CSR_DEF(sie)

R_CSR_DEF(sscratch)
W_CSR_DEF(sscratch)

#define CAUSE_INT_S_SOFTWARE 1
#define CAUSE_INT_M_SOFTWARE 3
#define CAUSE_INT_S_TIMER 5
#define CAUSE_INT_M_TIMER 7
#define CAUSE_INT_S_EXTERNAL 9
#define CAUSE_INT_M_EXTERNAL 11
#define CAUSE_INT_NUM 12
#define CAUSE_INT_OFFSET (-1UL / 2 + 1UL)

#define CAUSE_EXC_IF_MISALIGNED 0
#define CAUSE_EXC_IF_ACCESS_FAULT 1
#define CAUSE_EXC_ILLEGAL_INSTR 2
#define CAUSE_EXC_BREAKPOINT 3
#define CAUSE_EXC_LD_MISALIGNED 4
#define CAUSE_EXC_LD_ACCESS_FAULT 5
#define CAUSE_EXC_ST_MISALIGNED 6
#define CAUSE_EXC_ST_ACCESS_FAULT 7
#define CAUSE_EXC_U_ECALL 8
#define CAUSE_EXC_S_ECALL 9
#define CAUSE_EXC_M_ECALL 11
#define CAUSE_EXC_IF_PAGE_FAULT 12
#define CAUSE_EXC_LD_PAGE_FAULT 13
#define CAUSE_EXC_ST_PAGE_FAULT 15
#define CAUSE_EXC_NUM 16

R_CSR_DEF(scause)

enum rv64_satp_mode_t {
  BARE = 0,
  SV39 = 8,
  SV48 = 9,
};

typedef union {
  uint64_t val;
  struct {
    unsigned long ppn : 44;
    unsigned long asid : 16;
    enum rv64_satp_mode_t mode : 4;
  } __attribute__((packed)) bits;
} rv64_satp;

R_CSR_DEF(satp)
W_CSR_DEF(satp)

#undef R_REG_DEF
#undef W_REG_DEF
#undef R_CSR_DEF
#undef W_CSR_DEF

__attribute__((always_inline)) static inline unsigned long r_time(void) {
  unsigned long val;
  asm volatile("rdtime %0" : "=r"(val));
  return val;
}

#endif
#endif

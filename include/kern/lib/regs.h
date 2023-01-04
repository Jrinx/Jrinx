#ifndef _KERN_LIB_REGS_H_
#define _KERN_LIB_REGS_H_

#include <stdint.h>

#define R_REG_DEF(reg)                                                                         \
  static inline unsigned long r_##reg(void) {                                                  \
    unsigned long val;                                                                         \
    asm volatile("mv %0, " #reg : "=r"(val));                                                  \
    return val;                                                                                \
  }

#define W_REG_DEF(reg)                                                                         \
  static inline void w_##reg(unsigned long val) {                                              \
    asm volatile("mv " #reg ", %0" : : "r"(val));                                              \
  }

#define R_CSR_DEF(reg)                                                                         \
  static inline unsigned long csrr_##reg(void) {                                               \
    unsigned long val;                                                                         \
    asm volatile("csrr %0, " #reg : "=r"(val));                                                \
    return val;                                                                                \
  }

#define W_CSR_DEF(reg)                                                                         \
  static inline void csrw_##reg(unsigned long val) {                                           \
    asm volatile("csrw " #reg ", %0" : : "r"(val));                                            \
  }

R_REG_DEF(tp)
W_REG_DEF(tp)

#define CAUSE_INT_U_SOFTWARE 0
#define CAUSE_INT_S_SOFTWARE 1
#define CAUSE_INT_M_SOFTWARE 3
#define CAUSE_INT_U_TIMER 4
#define CAUSE_INT_S_TIMER 5
#define CAUSE_INT_M_TIMER 7
#define CAUSE_INT_U_EXTERNAL 8
#define CAUSE_INT_S_EXTERNAL 9
#define CAUSE_INT_M_EXTERNAL 11
#define CAUSE_INT_NUM 12
#define CAUSE_INT_OFFSET (-1UL / 2 + 1UL)

#define CAUSE_EXC_INSTR_ADDR_MISALIGNED 0
#define CAUSE_EXC_INSTR_ACCESS_FAULT 1
#define CAUSE_EXC_ILLEGAL_INSTR 2
#define CAUSE_EXC_BREAKPOINT 3
#define CAUSE_EXC_LOAD_ADDR_MISALIGNED 4
#define CAUSE_EXC_LOAD_ACCESS_FAULT 5
#define CAUSE_EXC_STORE_ADDR_MISALIGNED 6
#define CAUSE_EXC_STORE_ACCESS_FAULT 7
#define CAUSE_EXC_U_ECALL 8
#define CAUSE_EXC_S_ECALL 9
#define CAUSE_EXC_M_ECALL 11
#define CAUSE_EXC_INSTR_PAGE_FAULT 12
#define CAUSE_EXC_LOAD_PAGE_FAULT 13
#define CAUSE_EXC_STORE_PAGE_FAULT 15
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
  } bits;
} rv64_satp;

R_CSR_DEF(satp)
W_CSR_DEF(satp)

#undef R_REG_DEF
#undef W_REG_DEF

#endif

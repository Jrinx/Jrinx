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

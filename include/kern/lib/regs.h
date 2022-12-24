#ifndef _KERN_LIB_REGS_H_
#define _KERN_LIB_REGS_H_

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

R_REG_DEF(tp)
W_REG_DEF(tp)

#undef R_REG_DEF
#undef W_REG_DEF

#endif

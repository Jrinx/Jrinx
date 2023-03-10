#include <kern/lib/sbi.h>
#include <stdint.h>

static inline struct sbiret sbi_call(long arg0, long arg1, long arg2, long eid, long fid) {
  register long a0 asm("a0") = arg0;
  register long a1 asm("a1") = arg1;
  register long a2 asm("a2") = arg2;
  register long a6 asm("a6") = fid;
  register long a7 asm("a7") = eid;
  asm volatile("ecall"
               : "+r"(a0), "+r"(a1)
               : "r"(a0), "r"(a1), "r"(a2), "r"(a6), "r"(a7)
               : "memory");
  struct sbiret ret = {.error = a0, .value = a1};
  return ret;
}

inline long sbi_set_timer(uint64_t stime_value) {
  struct sbiret ret = sbi_call(stime_value, 0, 0, SBI_EXT_TIME_SET_TIMER, 0);
  return ret.error;
}

inline long sbi_console_putchar(int ch) {
  struct sbiret ret = sbi_call(ch, 0, 0, SBI_EXT_0_1_CONSOLE_PUTCHAR, 0);
  return ret.error;
}

inline long sbi_send_ipi(const unsigned long *hart_mask) {
  struct sbiret ret = sbi_call((long)hart_mask, 0, 0, SBI_EXT_0_1_SEND_IPI, 0);
  return ret.error;
}

inline void sbi_shutdown(void) {
  sbi_call(0, 0, 0, SBI_EXT_0_1_SHUTDOWN, 0);
  __builtin_unreachable();
}

inline struct sbiret sbi_hart_start(unsigned long hartid, unsigned long start_addr,
                                    unsigned long opaque) {
  return sbi_call(hartid, start_addr, opaque, SBI_EXT_HSM, SBI_EXT_HSM_HART_START);
}

inline struct sbiret sbi_hart_stop(void) {
  return sbi_call(0, 0, 0, SBI_EXT_HSM, SBI_EXT_HSM_HART_STOP);
}

inline struct sbiret sbi_system_reset(uint32_t reset_type, uint32_t reset_reason) {
  return sbi_call(reset_type, reset_reason, 0, SBI_EXT_SRST, SBI_EXT_SRST_RESET);
}

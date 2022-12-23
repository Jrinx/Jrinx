#include <kern/lib/ecall-intf.h>
#include <types.h>

inline long sbi_console_putchar(int ch) {
  register uint64_t a0 asm("a0") = ch;
  register uint64_t a7 asm("a7") = SBI_EXT_0_1_CONSOLE_PUTCHAR;
  asm volatile("ecall" : "+r"(a0) : "r"(a7) : "memory");
  return a0;
}

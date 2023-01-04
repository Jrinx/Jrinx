#include <kern/drivers/intc.h>
#include <kern/lib/debug.h>
#include <kern/lib/regs.h>
#include <stdint.h>

static trap_handler_t int_map[CAUSE_INT_NUM];
static trap_handler_t exc_map[CAUSE_EXC_NUM];
static irq_phandle_t phandle_map[1 << sizeof(uint8_t)];

const trap_handler_t *intc_get_int_vec(void) {
  return int_map;
}

const trap_handler_t *intc_get_exc_vec(void) {
  return exc_map;
}

void intc_register_handler(unsigned long trap_num, trap_handler_t handler) {
  if (trap_num > CAUSE_INT_OFFSET) {
    int_map[trap_num - CAUSE_INT_OFFSET] = handler;
  } else {
    exc_map[trap_num] = handler;
  }
}

irq_phandle_t intc_get_phandle(uint8_t phandle_num) {
  return phandle_map[phandle_num];
}

void intc_set_phandle(uint8_t phandle_num, irq_phandle_t func) {
  phandle_map[phandle_num] = func;
}

#include <kern/drivers/intc.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lib/regs.h>
#include <stdint.h>

static trap_callback_t exc_map[CAUSE_EXC_NUM];
static trap_callback_t int_map[CAUSE_INT_NUM];

static irq_register_callback_t phandle_map[1U << (sizeof(uint8_t) * 8)];

const trap_callback_t *intc_get_exc_vec(void) {
  return exc_map;
}

long intc_register_handler(void *_, unsigned long trap_num, trap_callback_t callback) {
  if (trap_num > CAUSE_INT_OFFSET) {
    int_map[trap_num - CAUSE_INT_OFFSET] = callback;
    csrw_sie(csrr_sie() | (1 << (trap_num - CAUSE_INT_OFFSET)));
  } else {
    exc_map[trap_num] = callback;
  }
  return KER_SUCCESS;
}

void intc_get_register_func(uint32_t phandle_num, irq_register_callback_t *callback) {
  assert(phandle_num < sizeof(phandle_map) / sizeof(irq_register_callback_t));
  *callback = phandle_map[phandle_num];
}

void intc_set_register_func(uint32_t phandle_num, irq_register_callback_t callback) {
  assert(phandle_num < sizeof(phandle_map) / sizeof(irq_register_callback_t));
  phandle_map[phandle_num] = callback;
}

#include <kern/drivers/intc.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lib/regs.h>
#include <stdint.h>

static trap_handler_t exc_map[CAUSE_EXC_NUM];

static trap_handler_t int_map[CAUSE_INT_NUM];
static void *int_ctx_map[CAUSE_INT_NUM];

static irq_phandle_t phandle_map[1U << (sizeof(uint8_t) * 8)];
static void *phandle_ctx_map[1U << (sizeof(uint8_t) * 8)];

const trap_handler_t *intc_get_exc_vec(void) {
  return exc_map;
}

long intc_register_handler(void *_, unsigned long trap_num, trap_handler_t handler,
                           void *subctx) {
  if (trap_num > CAUSE_INT_OFFSET) {
    int_map[trap_num - CAUSE_INT_OFFSET] = handler;
    int_ctx_map[trap_num - CAUSE_INT_OFFSET] = subctx;
  } else {
    exc_map[trap_num] = handler;
  }
  return KER_SUCCESS;
}

irq_phandle_t intc_get_phandle(uint32_t phandle_num, void **ctx) {
  assert(phandle_num < (1U << (sizeof(uint8_t) * 8)));
  *ctx = phandle_ctx_map[phandle_num];
  return phandle_map[phandle_num];
}

void intc_set_phandle(uint32_t phandle_num, irq_phandle_t func, void *ctx) {
  assert(phandle_num < (1U << (sizeof(uint8_t) * 8)));
  phandle_map[phandle_num] = func;
  phandle_ctx_map[phandle_num] = ctx;
}

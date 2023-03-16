#include <kern/drivers/intc.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lib/regs.h>
#include <kern/mm/kalloc.h>
#include <lib/hashmap.h>
#include <lib/string.h>
#include <stdint.h>

struct exc_st {
  uint32_t exc_trap_num;
  trap_callback_t exc_callback;
  struct linked_node exc_link;
};

static const void *exc_key_of(const struct linked_node *node) {
  const struct exc_st *exc = CONTAINER_OF(node, struct exc_st, exc_link);
  return &exc->exc_trap_num;
}

static struct hlist_head exc_map_array[64];
static struct hashmap exc_map = {
    .h_array = exc_map_array,
    .h_num = 0,
    .h_cap = 64,
    .h_code = hash_code_uint32,
    .h_equals = hash_eq_uint32,
    .h_key = exc_key_of,
};

struct irq_reg_st {
  uint32_t irq_phandle_num;
  irq_register_callback_t irq_reg_callback;
  struct linked_node irq_link;
};

static const void *irq_reg_key_of(const struct linked_node *node) {
  const struct irq_reg_st *irq_reg = CONTAINER_OF(node, struct irq_reg_st, irq_link);
  return &irq_reg->irq_phandle_num;
}

static struct hlist_head irq_reg_map_array[16];
static struct hashmap irq_reg_map = {
    .h_array = irq_reg_map_array,
    .h_num = 0,
    .h_cap = 16,
    .h_code = hash_code_uint32,
    .h_equals = hash_eq_uint32,
    .h_key = irq_reg_key_of,
};

long intc_register_handler(void *_, unsigned long trap_num, trap_callback_t callback) {
  struct exc_st *exc = kalloc(sizeof(struct exc_st));
  exc->exc_trap_num = trap_num;
  exc->exc_callback = callback;
  hashmap_put(&exc_map, &exc->exc_link);
  return KER_SUCCESS;
}

void intc_get_irq_reg(uint32_t phandle_num, irq_register_callback_t *callback) {
  struct linked_node *node = hashmap_get(&irq_reg_map, &phandle_num);
  if (node == NULL) {
    memset(callback, 0, sizeof(irq_register_callback_t));
    return;
  }
  struct irq_reg_st *irq_reg = CONTAINER_OF(node, struct irq_reg_st, irq_link);
  memcpy(callback, &irq_reg->irq_reg_callback, sizeof(irq_register_callback_t));
}

void intc_register_irq_reg(uint32_t phandle_num, irq_register_callback_t callback) {
  struct irq_reg_st *irq_reg = kalloc(sizeof(struct irq_reg_st));
  irq_reg->irq_phandle_num = phandle_num;
  irq_reg->irq_reg_callback = callback;
  hashmap_put(&irq_reg_map, &irq_reg->irq_link);
}

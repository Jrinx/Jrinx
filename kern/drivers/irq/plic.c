#include "plic.h"
#include <endian.h>
#include <kern/drivers/device.h>
#include <kern/drivers/intc.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lock/lock.h>
#include <kern/lock/spinlock.h>
#include <kern/mm/kalloc.h>
#include <kern/mm/vmm.h>
#include <layouts.h>
#include <lib/string.h>

struct plic {
  char *pl_name;
  uintptr_t pl_addr;
  uintmax_t pl_size;
  trap_callback_t pl_int_map[PLIC_SOURCE_MAX + 1];
  struct lock spinlock_of(pl);
};

static unsigned int external_int_ctx;

static void plic_set_source_prio(struct plic *plic, uint32_t source_id, uint32_t priority) {
  *((volatile uint32_t *)(plic->pl_addr + 4 * source_id)) = priority;
}

static void plic_source_set_en(struct plic *plic, uint32_t context_id, uint32_t source_id,
                               int enable) {
  unsigned long ctx_base = plic->pl_addr + 0x2000 + context_id * 0x80;
  uint32_t quo = source_id / (sizeof(uint32_t) * 8);
  uint32_t rem = source_id % (sizeof(uint32_t) * 8);
  uint32_t bits = *((volatile uint32_t *)(ctx_base + quo * 4));
  if (enable) {
    bits |= 1U << rem;
  } else {
    bits &= ~(1U << rem);
  }
  *((volatile uint32_t *)(ctx_base + quo * 4)) = bits;
}

static void plic_source_disable_all(struct plic *plic, uint32_t context_id) {
  unsigned long ctx_base = plic->pl_addr + 0x2000 + context_id * 0x80;
  memset((void *)ctx_base, 0, 0x80);
}

static void plic_set_context_prio_threshold(struct plic *plic, uint32_t context_id,
                                            uint32_t threshold) {
  *((volatile uint32_t *)(plic->pl_addr + 0x200000 + 0x1000 * context_id)) = threshold;
}

static uint32_t plic_claim(struct plic *plic, uint32_t context_id) {
  return *((volatile uint32_t *)(plic->pl_addr + 0x200004 + 0x1000 * context_id));
}

static void plic_complete(struct plic *plic, uint32_t context_id, uint32_t int_num) {
  *((volatile uint32_t *)(plic->pl_addr + 0x200004 + 0x1000 * context_id)) = int_num;
}

static long plic_handle_int(void *ctx, unsigned long trap_num) {
  struct plic *plic = ctx;

  panic_e(lk_acquire(&plic->spinlock_of(pl)));

  uint32_t int_num = plic_claim(plic, external_int_ctx);
  if (int_num < PLIC_SOURCE_MIN || int_num > PLIC_SOURCE_MAX) {
    panic_e(lk_release(&plic->spinlock_of(pl)));
    return -KER_INT_ER;
  }

  trap_callback_t callback = plic->pl_int_map[int_num];

  catch_e(cb_invoke(callback)(int_num), {
    panic_e(lk_release(&plic->spinlock_of(pl)));
    return err;
  });

  plic_complete(plic, external_int_ctx, int_num);

  panic_e(lk_release(&plic->spinlock_of(pl)));
  return KER_SUCCESS;
}

static long plic_register_irq(void *ctx, unsigned long source_id, trap_callback_t callback) {
  if (source_id < PLIC_SOURCE_MIN || source_id > PLIC_SOURCE_MAX) {
    return -KER_INT_ER;
  }
  struct plic *plic = ctx;
  panic_e(lk_acquire(&plic->spinlock_of(pl)));
  plic_source_set_en(plic, external_int_ctx, source_id, 1);
  plic_set_source_prio(plic, source_id, PLIC_PRIO_MAX);
  plic->pl_int_map[source_id] = callback;
  panic_e(lk_release(&plic->spinlock_of(pl)));
  return KER_SUCCESS;
}

static void plic_init(struct plic *plic) {
  info("set all interrupt sources priority to MIN\n");
  for (uint32_t i = PLIC_SOURCE_MIN; i < PLIC_SOURCE_MAX; i++) {
    plic_set_source_prio(plic, i, PLIC_PRIO_MIN);
  }

  uint32_t context_max_id = (plic->pl_size - 0x200000UL) / 0x1000UL - 1UL;
  assert(context_max_id >= external_int_ctx);
  info("disable all interrupt sources for all context (0 - %d)\n", context_max_id);
  info("set all context priority threshold to MAX\n");
  for (uint32_t i = 0; i <= context_max_id; i++) {
    plic_source_disable_all(plic, i);
    plic_set_context_prio_threshold(plic, i, PLIC_PRIO_MAX);
  }

  info("all interrupt sources shall be handled by context %u\n", external_int_ctx);
  plic_set_context_prio_threshold(plic, external_int_ctx, PLIC_PRIO_MIN);
}

static int plic_pred(const struct dev_node *node) {
  struct dev_node_prop *prop = dt_node_prop_extract(node, "compatible");
  return prop != NULL &&
         (dt_match_strlist(prop->pr_values, prop->pr_len, "sifive,plic-1.0.0") ||
          dt_match_strlist(prop->pr_values, prop->pr_len, "riscv,plic0"));
}

static long plic_probe(const struct dev_node *node) {
  struct dev_node_prop *prop;
  prop = dt_node_prop_extract(node, "reg");
  if (prop == NULL) {
    return -KER_DTB_ER;
  }
  uint64_t addr = from_be(*((uint64_t *)prop->pr_values));
  uint64_t size = from_be(*((uint64_t *)prop->pr_values + 1));

  prop = dt_node_prop_extract(node, "phandle");
  if (prop == NULL) {
    return -KER_DEV_ER;
  }

  extern const char *boot_dt_model;
  if (boot_dt_model == NULL) {
    return -KER_DEV_ER;
  }
  if (strcmp(boot_dt_model, "riscv-virtio,qemu") == 0) {
    external_int_ctx = SYSCORE * 2 + 1;
  } else if (strcmp(boot_dt_model, "SiFive HiFive Unleashed A00") == 0 ||
             strcmp(boot_dt_model, "SiFive HiFive Unmatched A00") == 0) {
    external_int_ctx = SYSCORE * 2;
  }

  struct plic *plic = kalloc(sizeof(struct plic));
  plic->pl_name = node->nd_name;
  plic->pl_addr = addr;
  plic->pl_size = size;
  spinlock_init(&plic->spinlock_of(pl));
  memset(plic->pl_int_map, 0, sizeof(plic->pl_int_map));

  unsigned int phandle = from_be(*((uint32_t *)prop->pr_values));
  info("%s probed (phandle: %u) to handle user external int\n", node->nd_name, phandle);
  struct fmt_mem_range mem_range = {.addr = addr, .size = size};
  info("\tlocates at %pM (size: %pB)\n", &mem_range, &size);

  cb_decl(trap_callback_t, trap_callback, plic_handle_int, plic);
  catch_e(intc_register_handler(NULL, CAUSE_INT_OFFSET + CAUSE_INT_S_EXTERNAL, trap_callback));
  cb_decl(irq_register_callback_t, irq_register_callback, plic_register_irq, plic);
  intc_register_irq_reg(phandle, irq_register_callback);

  plic_init(plic);
  vmm_register_mmio(plic->pl_name, &plic->pl_addr, plic->pl_size);

  return KER_SUCCESS;
}

static struct device plic_device = {
    .d_pred = plic_pred,
    .d_probe = plic_probe,
};

device_init(plic_device, medium);

#include <endian.h>
#include <kern/drivers/device.h>
#include <kern/drivers/intc.h>
#include <kern/drivers/irq/plic.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lock/lock.h>
#include <kern/lock/spinlock.h>
#include <kern/mm/pmm.h>
#include <kern/mm/vmm.h>
#include <lib/string.h>

struct plic {
  char *pl_name;
  unsigned long pl_addr;
  unsigned long pl_size;
  trap_callback_t pl_int_map[PLIC_SOURCE_MAX + 1];
  struct lock spinlock_of(pl);
  LIST_ENTRY(plic) pl_link;
};

static LIST_HEAD(, plic) plic_list;

static void plic_set_source_prio(struct plic *plic, uint32_t source_id, uint32_t priority) {
  *((volatile uint32_t *)(plic->pl_addr + 4 * source_id)) = priority;
}

static void plic_source_enable(struct plic *plic, uint32_t context_id, uint32_t source_id) {
  unsigned long ctx_base = plic->pl_addr + 0x2000 + context_id * 0x80;
  uint32_t quo = source_id / (sizeof(uint32_t) * 8);
  uint32_t rem = source_id % (sizeof(uint32_t) * 8);
  uint32_t bits = *((volatile uint32_t *)(ctx_base + quo * 4));
  bits |= 1U << rem;
  *((volatile uint32_t *)(ctx_base + quo * 4)) = bits;
}

static void plic_source_disable(struct plic *plic, uint32_t context_id, uint32_t source_id) {
  unsigned long ctx_base = plic->pl_addr + 0x2000 + context_id * 0x80;
  uint32_t quo = source_id / (sizeof(uint32_t) * 8);
  uint32_t rem = source_id % (sizeof(uint32_t) * 8);
  uint32_t bits = *((volatile uint32_t *)(ctx_base + quo * 4));
  bits &= ~(1U << rem);
  *((volatile uint32_t *)(ctx_base + quo * 4)) = bits;
}

static void plic_set_context_prio_threshold(struct plic *plic, uint32_t context_id,
                                            uint32_t threshold) {
  *((volatile uint32_t *)(plic->pl_addr + 0x200000 + 0x1000 * context_id)) = threshold;
}

static uint32_t plic_claim(struct plic *plic, uint32_t context_id) {
  return *((volatile uint32_t *)(plic->pl_addr + 0x200004 + 0x1000 * context_id));
}

static void plic_complete(struct plic *plic, uint32_t context_id) {
  *((volatile uint32_t *)(plic->pl_addr + 0x200004 + 0x1000 * context_id)) = 0;
}

static long plic_handle_int(void *ctx, unsigned long trap_num) {
  struct plic *plic = ctx;

  catch_e(lk_acquire(&plic->spinlock_of(pl)));

  uint32_t int_num = plic_claim(plic, PLIC_EXTERNAL_INT_CTX);
  if (int_num < PLIC_SOURCE_MIN || int_num > PLIC_SOURCE_MAX) {
    catch_e(lk_release(&plic->spinlock_of(pl)));
    return -KER_INT_ER;
  }

  trap_callback_t callback = plic->pl_int_map[int_num];

  catch_e(cb_invoke(callback)(int_num), {
    catch_e(lk_release(&plic->spinlock_of(pl)));
    return err;
  });

  plic_complete(plic, PLIC_EXTERNAL_INT_CTX);

  catch_e(lk_release(&plic->spinlock_of(pl)));
  return KER_SUCCESS;
}

static long plic_register_irq(void *ctx, unsigned long source_id, trap_callback_t callback) {
  if (source_id < PLIC_SOURCE_MIN || source_id > PLIC_SOURCE_MAX) {
    return -KER_INT_ER;
  }
  struct plic *plic = ctx;
  catch_e(lk_acquire(&plic->spinlock_of(pl)));
  plic_source_enable(plic, PLIC_EXTERNAL_INT_CTX, source_id);
  plic_set_source_prio(plic, source_id, PLIC_PRIO_MAX);
  plic->pl_int_map[source_id] = callback;
  catch_e(lk_release(&plic->spinlock_of(pl)));
  return KER_SUCCESS;
}

static void plic_init(struct plic *plic) {
  for (uint32_t i = PLIC_SOURCE_MIN; i < PLIC_SOURCE_MAX; i++) {
    plic_set_source_prio(plic, i, PLIC_PRIO_MIN);
  }

  uint32_t context_max_id = (plic->pl_size - 0x200000UL) / 0x1000UL - 1UL;
  assert(context_max_id >= PLIC_EXTERNAL_INT_CTX);
  for (uint32_t i = 0; i <= context_max_id; i++) {
    for (uint32_t j = PLIC_SOURCE_MIN; j <= PLIC_SOURCE_MAX; j++) {
      plic_source_disable(plic, i, j);
    }
    plic_set_context_prio_threshold(plic, i, PLIC_PRIO_MAX);
  }

  info("all external interrupt shall be handled by context %u\n", PLIC_EXTERNAL_INT_CTX);
  plic_set_context_prio_threshold(plic, PLIC_EXTERNAL_INT_CTX, PLIC_PRIO_MIN);
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
  unsigned long addr = from_be(*((uint64_t *)prop->pr_values));
  unsigned long size = from_be(*((uint64_t *)prop->pr_values + 1));

  prop = dt_node_prop_extract(node, "phandle");
  if (prop == NULL) {
    return -KER_DEV_ER;
  }

  struct plic *plic = alloc(sizeof(struct plic), sizeof(struct plic));
  plic->pl_name = node->nd_name;
  plic->pl_addr = addr;
  plic->pl_size = size;
  spinlock_init(&plic->spinlock_of(pl));
  memset(plic->pl_int_map, 0, sizeof(plic->pl_int_map));
  LIST_INSERT_HEAD(&plic_list, plic, pl_link);

  unsigned int phandle = from_be(*((uint32_t *)prop->pr_values));
  info("%s probed (phandle: %u) to handle user external int\n", node->nd_name, phandle);
  info("\tlocates at ");
  mem_print_range(addr, size, NULL);

  cb_decl(trap_callback_t, trap_callback, plic_handle_int, plic);
  catch_e(intc_register_handler(NULL, CAUSE_INT_OFFSET + CAUSE_INT_S_EXTERNAL, trap_callback));
  cb_decl(irq_register_callback_t, irq_register_callback, plic_register_irq, plic);
  intc_set_register_func(phandle, irq_register_callback);

  plic_init(plic);
  vmm_register_mmio(plic->pl_name, &plic->pl_addr, plic->pl_size);

  return KER_SUCCESS;
}

struct device plic_device = {
    .d_pred = plic_pred,
    .d_probe = plic_probe,
    .d_probe_pri = MEDIUM,
};

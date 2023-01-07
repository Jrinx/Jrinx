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
#include <layouts.h>
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

static long plic_setup_map(void *ctx) {
  struct plic *plic = ctx;
  unsigned long addr = plic->pl_addr;
  unsigned long size = plic->pl_size;
  vaddr_t va = {.val = addr + DEVOFFSET};
  paddr_t pa = {.val = addr};
  perm_t perm = {.bits = {.a = 1, .d = 1, .r = 1, .w = 1, .g = 1}};
  info("set up %s mapping at ", plic->pl_name);
  mem_print_range(addr + DEVOFFSET, size, NULL);
  for (; va.val < addr + DEVOFFSET + size; va.val += PGSIZE, pa.val += PGSIZE) {
    catch_e(pt_map(kern_pgdir, va, pa, perm));
  }
  plic->pl_addr += DEVOFFSET;
  return KER_SUCCESS;
}

static void plic_set_source_prio(struct plic *plic, uint8_t source_id, uint32_t priority) {
  *((volatile uint32_t *)(plic->pl_addr + 4 * source_id)) = priority;
}

static uint64_t plic_get_pending_bits(struct plic *plic) {
  uint64_t lo = *((volatile uint32_t *)(plic->pl_addr + 0x1000));
  uint64_t hi = *((volatile uint32_t *)(plic->pl_addr + 0x1004));
  return (hi << 32) | lo;
}

static void plic_set_source_enable(struct plic *plic, uint8_t context_id, uint64_t enable) {
  if (context_id == 0) {
    *((volatile uint32_t *)(plic->pl_addr + 0x2000)) = enable & 0xffffffffUL;
  } else {
    *((volatile uint32_t *)(plic->pl_addr + 0x2000 + 0x80 * context_id)) =
        enable & 0xffffffffUL;
    *((volatile uint32_t *)(plic->pl_addr + 0x2004 + 0x80 * context_id)) = enable >> 32;
  }
}

static uint64_t plic_get_source_enable(struct plic *plic, uint8_t context_id) {
  if (context_id == 0) {
    return *((volatile uint32_t *)(plic->pl_addr + 0x2000));
  } else {
    uint64_t lo = *((volatile uint32_t *)(plic->pl_addr + 0x2000 + 0x80 * context_id));
    uint64_t hi = *((volatile uint32_t *)(plic->pl_addr + 0x2004 + 0x80 * context_id));
    return (hi << 32) | lo;
  }
}

static void plic_set_context_prio_threshold(struct plic *plic, uint8_t context_id,
                                            enum plic_context_mode_t mode, uint32_t threshold) {
  if (context_id == 0) {
    *((volatile uint32_t *)(plic->pl_addr + 0x200000)) = threshold;
  } else {
    if (mode == M_MODE) {
      *((volatile uint32_t *)(plic->pl_addr + 0x201000 + 0x2000 * (context_id - 1))) =
          threshold;
    } else {
      *((volatile uint32_t *)(plic->pl_addr + 0x200000 + 0x2000 * context_id)) = threshold;
    }
  }
}

static uint32_t plic_claim(struct plic *plic, uint8_t context_id,
                           enum plic_context_mode_t mode) {
  if (context_id == 0) {
    return *((volatile uint32_t *)(plic->pl_addr + 0x200004));
  } else {
    if (mode == M_MODE) {
      return *((volatile uint32_t *)(plic->pl_addr + 0x201004 + 0x2000 * (context_id - 1)));
    } else {
      return *((volatile uint32_t *)(plic->pl_addr + 0x200004 + 0x2000 * context_id));
    }
  }
}

static void plic_complete(struct plic *plic, uint8_t context_id,
                          enum plic_context_mode_t mode) {
  if (context_id == 0) {
    *((volatile uint32_t *)(plic->pl_addr + 0x200004)) = 0;
  } else {
    if (mode == M_MODE) {
      *((volatile uint32_t *)(plic->pl_addr + 0x201004 + 0x2000 * (context_id - 1))) = 0;
    } else {
      *((volatile uint32_t *)(plic->pl_addr + 0x200004 + 0x2000 * context_id)) = 0;
    }
  }
}

static long plic_handle_int(void *ctx, unsigned long trap_num) {
  struct plic *plic = ctx;

  catch_e(lk_acquire(&plic->spinlock_of(pl)));

  uint64_t pending_bits = plic_get_pending_bits(plic);
  if (pending_bits == 0) {
    catch_e(lk_release(&plic->spinlock_of(pl)));
    return -KER_INT_ER;
  }

  uint32_t int_num = plic_claim(plic, PLIC_EXTERNAL_INT_CTX, M_MODE);
  if (int_num < PLIC_SOURCE_MIN || int_num > PLIC_SOURCE_MAX) {
    catch_e(lk_release(&plic->spinlock_of(pl)));
    return -KER_INT_ER;
  }

  trap_callback_t callback = plic->pl_int_map[int_num];

  catch_e(cb_invoke(callback)(int_num), {
    catch_e(lk_release(&plic->spinlock_of(pl)));
    return err;
  });

  plic_complete(plic, PLIC_EXTERNAL_INT_CTX, M_MODE);

  catch_e(lk_release(&plic->spinlock_of(pl)));
  return KER_SUCCESS;
}

static long plic_register_irq(void *ctx, unsigned long int_num, trap_callback_t callback) {
  if (int_num < PLIC_SOURCE_MIN || int_num > PLIC_SOURCE_MAX) {
    return -KER_INT_ER;
  }
  struct plic *plic = ctx;
  catch_e(lk_acquire(&plic->spinlock_of(pl)));
  plic_set_source_enable(plic, PLIC_EXTERNAL_INT_CTX,
                         plic_get_source_enable(plic, PLIC_EXTERNAL_INT_CTX) | (1 << int_num));
  plic_set_source_prio(plic, int_num, PLIC_PRIO_MAX);
  plic->pl_int_map[int_num] = callback;
  catch_e(lk_release(&plic->spinlock_of(pl)));
  return KER_SUCCESS;
}

static void plic_init(struct plic *plic) {
  for (uint8_t i = PLIC_SOURCE_MIN; i < PLIC_SOURCE_MAX; i++) {
    plic_set_source_prio(plic, i, PLIC_PRIO_MIN);
  }

  for (uint8_t i = PLIC_CONTEXT_MIN; i < PLIC_CONTEXT_MAX; i++) {
    plic_set_source_enable(plic, i, 0);
    if (i != 0) {
      plic_set_context_prio_threshold(plic, i, M_MODE, PLIC_PRIO_MAX);
    }
    plic_set_context_prio_threshold(plic, i, S_MODE, PLIC_PRIO_MAX);
  }

  info("all external interrupt shall be handled by context %u\n", PLIC_EXTERNAL_INT_CTX);
  plic_set_context_prio_threshold(plic, PLIC_EXTERNAL_INT_CTX, M_MODE, PLIC_PRIO_MIN);
}

static long plic_probe(const struct dev_node *node) {
  struct dev_node_prop *prop;
  prop = dt_node_prop_extract(node, "compatible");
  if (prop == NULL || !(dt_match_strlist(prop->pr_values, prop->pr_len, "sifive,plic-1.0.0") ||
                        dt_match_strlist(prop->pr_values, prop->pr_len, "riscv,plic0"))) {
    return KER_SUCCESS;
  }

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
  cb_decl(mmio_setup_callback_t, plic_setup_callback, plic_setup_map, plic);
  vmm_register_mmio(plic_setup_callback);

  return KER_SUCCESS;
}

struct device plic_device = {
    .d_probe = plic_probe,
    .d_probe_pri = MEDIUM,
};

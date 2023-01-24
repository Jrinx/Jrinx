#include <endian.h>
#include <kern/drivers/device.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lib/hart.h>
#include <kern/lib/sbi.h>
#include <kern/mm/pmm.h>
#include <layouts.h>
#include <lib/string.h>

static uint8_t kern_master_stack[KSTKSIZE]
    __attribute__((aligned(PGSIZE), section(".ksec.master_stack")));

const uint8_t *kern_master_stacktop = kern_master_stack + KSTKSIZE;

unsigned long *cpus_stacktop = NULL;

static unsigned long cpus_count;

unsigned long cpus_get_count(void) {
  return cpus_count;
}

static int cpus_pred(const struct dev_node *node) {
  return strcmp(node->nd_name, "cpus") == 0;
}

static int cpus_check(const struct dev_node *node) {
  struct dev_node_prop *prop;
  prop = dt_node_prop_extract(node, "device_type");
  if (prop == NULL || strcmp((char *)prop->pr_values, "cpu") != 0) {
    return 0;
  }
  prop = dt_node_prop_extract(node, "compatible");
  if (prop == NULL || !dt_match_strlist(prop->pr_values, prop->pr_len, "riscv")) {
    return 0;
  }
  prop = dt_node_prop_extract(node, "mmu-type");
  if (prop == NULL) {
    return 0;
  }
  return 1;
}

static long cpus_probe(const struct dev_node *node) {
  cpus_count = 0;
  struct dev_node *child;
  LINKED_NODE_ITER (node->nd_children_list.l_first, child, nd_link) {
    if (cpus_check(child)) {
      cpus_count++;
    }
  }

  cpus_stacktop = alloc(sizeof(unsigned long) * cpus_count, sizeof(unsigned long));

  LINKED_NODE_ITER (node->nd_children_list.l_first, child, nd_link) {
    if (!cpus_check(child)) {
      continue;
    }
    struct dev_node_prop *prop;
    prop = dt_node_prop_extract(child, "reg");
    if (prop == NULL) {
      return -KER_DTB_ER;
    }
    unsigned long id;
    switch (prop->pr_len) {
    case sizeof(uint32_t):
      id = from_be(*((uint32_t *)prop->pr_values));
      break;
    case sizeof(uint64_t):
      id = from_be(*((uint64_t *)prop->pr_values));
      break;
    default:
      return -KER_DTB_ER;
    }
    if (id != hrt_get_id()) {
      unsigned long stack_top = (unsigned long)alloc(KSTKSIZE, PGSIZE) + KSTKSIZE;
      cpus_stacktop[id] = stack_top;
      info("%s (slave)  probed (stack top: %016lx)\n", child->nd_name, stack_top);
      catch_e(sbi_hart_start(id, KERNBASE, 0));
    } else {
      cpus_stacktop[id] = (unsigned long)kern_master_stacktop;
      info("%s (master) probed (stack top: %016lx)\n", child->nd_name,
           (unsigned long)kern_master_stacktop);
    }
  }
  return KER_SUCCESS;
}

static struct device cpus_device = {
    .d_pred = cpus_pred,
    .d_probe = cpus_probe,
};

device_init(cpus_device, high);

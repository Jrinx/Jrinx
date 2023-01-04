#include <endian.h>
#include <kern/drivers/device.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/mm/pmm.h>
#include <lib/string.h>

static size_t mem_num;
static uint64_t *mem_addr;
static uint64_t *mem_size;

static long mem_probe(const struct dev_node *node) {
  struct dev_node_prop *prop;
  prop = dt_node_prop_extract(node, "device_type");
  if (prop == NULL || strcmp((char *)prop->pr_values, "memory") != 0) {
    return KER_SUCCESS;
  }

  prop = dt_node_prop_extract(node, "reg");
  if (prop == NULL) {
    return -KER_DTB_ER;
  }

  if (prop->pr_len % (sizeof(uint64_t) * 2) != 0) {
    return -KER_DTB_ER;
  }
  mem_num = prop->pr_len / (sizeof(uint64_t) * 2);
  mem_addr = alloc(sizeof(uint64_t) * mem_num, sizeof(uint64_t));
  mem_size = alloc(sizeof(uint64_t) * mem_num, sizeof(uint64_t));
  uint64_t *reg_table = (uint64_t *)prop->pr_values;
  for (size_t i = 0; i < mem_num; i++) {
    mem_addr[i] = from_be(reg_table[i * 2]);
    mem_size[i] = from_be(reg_table[i * 2 + 1]);
  }

  info("%s probed (consists of %lu memory):\n", node->nd_name, mem_num);

  for (size_t i = 0; i < mem_num; i++) {
    info("\tmemory[%lu] locates at ", i);
    mem_print_range(mem_addr[i], mem_size[i], NULL);
  }

  pmm_init();

  return KER_SUCCESS;
}

size_t mem_get_num(void) {
  return mem_num;
}

long mem_get_addr(unsigned i, uint64_t *addr) {
  if (i >= mem_num) {
    return -KER_MEM_ER;
  }
  *addr = mem_addr[i];
  return KER_SUCCESS;
}

long mem_get_size(unsigned i, uint64_t *size) {
  if (i >= mem_num) {
    return -KER_MEM_ER;
  }
  *size = mem_size[i];
  return KER_SUCCESS;
}

struct device memory_device = {
    .d_probe = mem_probe,
    .d_probe_pri = HIGHEST,
};

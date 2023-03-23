#include <endian.h>
#include <kern/drivers/device.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/mm/kalloc.h>
#include <lib/string.h>

static size_t mem_num;
static uintptr_t *mem_addr;
static uintmax_t *mem_size;

static int mem_pred(const struct dev_node *node) {
  struct dev_node_prop *prop = dt_node_prop_extract(node, "device_type");
  return prop != NULL && strcmp((char *)prop->pr_values, "memory") == 0;
}

static long mem_probe(const struct dev_node *node) {
  struct dev_node_prop *prop;
  prop = dt_node_prop_extract(node, "reg");
  if (prop == NULL) {
    return -KER_DTB_ER;
  }

  if (prop->pr_len % (sizeof(uint64_t) * 2) != 0) {
    return -KER_DTB_ER;
  }
  mem_num = prop->pr_len / (sizeof(uint64_t) * 2);
  mem_addr = kalloc(sizeof(uintptr_t) * mem_num);
  mem_size = kalloc(sizeof(uintmax_t) * mem_num);
  uint64_t *reg_table = (uint64_t *)prop->pr_values;
  for (size_t i = 0; i < mem_num; i++) {
    mem_addr[i] = from_be(reg_table[i * 2]);
    mem_size[i] = from_be(reg_table[i * 2 + 1]);
  }

  info("%s probed (consists of %lu memory):\n", node->nd_name, mem_num);

  for (size_t i = 0; i < mem_num; i++) {
    struct fmt_mem_range mem_range = {.addr = mem_addr[i], .size = mem_size[i]};
    info("\tmemory[%lu] locates at %pM (size: %pB)\n", i, &mem_range, &mem_size[i]);
  }

  return KER_SUCCESS;
}

size_t mem_get_num(void) {
  return mem_num;
}

long mem_get_addr(unsigned i, uintptr_t *addr) {
  if (i >= mem_num) {
    return -KER_MEM_ER;
  }
  *addr = mem_addr[i];
  return KER_SUCCESS;
}

long mem_get_size(unsigned i, uintmax_t *size) {
  if (i >= mem_num) {
    return -KER_MEM_ER;
  }
  *size = mem_size[i];
  return KER_SUCCESS;
}

static struct device memory_device = {
    .d_pred = mem_pred,
    .d_probe = mem_probe,
};

device_init(memory_device, high);

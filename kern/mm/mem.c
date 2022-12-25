#include <brpred.h>
#include <endian.h>
#include <kern/drivers/device.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <lib/string.h>
#include <stddef.h>
#include <stdint.h>

static size_t mem_num;
static uint64_t *mem_addr;
static uint64_t *mem_size;

static unsigned long freemem_base = 0;

void *bare_alloc(size_t size) {
  if (unlikely(freemem_base == 0)) {
    extern uint8_t kern_end[];
    freemem_base = (unsigned long)kern_end;
  }

  freemem_base = freemem_base / size * size + size;

  void *res = (void *)freemem_base;

  freemem_base += size;

  return res;
}

static long mem_probe(const struct dev_node *node) {
  int reg_found = 0;
  struct dev_node_prop *prop;
  TAILQ_FOREACH (prop, &node->nd_prop_tailq, pr_link) {
    if (strcmp(prop->pr_name, "reg") == 0) {
      reg_found = 1;
      if (prop->pr_len % (sizeof(uint64_t) * 2) != 0) {
        return -KER_DTB_ER;
      }
      mem_num = prop->pr_len / (sizeof(uint64_t) * 2);
      mem_addr = bare_alloc(sizeof(uint64_t) * mem_num);
      mem_size = bare_alloc(sizeof(uint64_t) * mem_num);
      uint64_t *reg_table = (uint64_t *)prop->pr_values;
      for (size_t i = 0; i < mem_num; i++) {
        mem_addr[i] = from_be(reg_table[i * 2]);
        mem_size[i] = from_be(reg_table[i * 2 + 1]);
      }
      break;
    }
  }

  if (!reg_found) {
    return -KER_DTB_ER;
  }

  return KER_SUCCESS;
}

struct device memory_device = {
    .d_name = "memory",
    .d_probe = mem_probe,
    .d_probe_pri = HIGHEST,
};

void memory_init(void) {
  info("%lu memory probed\n", mem_num);

  for (size_t i = 0; i < mem_num; i++) {
    info("memory[%lu] is [%016lx, %016lx) <size=%lu MiB>\n", i, mem_addr[i],
         mem_addr[i] + mem_size[i], mem_size[i] / (1 << 20));
  }
}

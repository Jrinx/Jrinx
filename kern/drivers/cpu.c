#include <endian.h>
#include <kern/drivers/device.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lib/hart.h>
#include <kern/lib/sbi.h>
#include <kern/mm/mem.h>
#include <layouts.h>
#include <lib/string.h>

int cpu_master_init = 0;
static unsigned long cpu_count = 0;

static long cpu_probe(const struct dev_node *node) {
  int reg_found = 0;
  struct dev_node_prop *prop;
  TAILQ_FOREACH (prop, &node->nd_prop_tailq, pr_link) {
    if (strcmp(prop->pr_name, "reg") == 0) {
      reg_found = 1;
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
        unsigned long stack_top = (unsigned long)alloc(KSTKSIZE, PGSIZE);
        info("%s (slave)  probed (stack top: %016lx)\n", node->nd_name, stack_top);
        unsigned long hart_mask[id / (sizeof(unsigned long) * 8) + 1];
        memset(hart_mask, 0, (id / (sizeof(unsigned long) * 8) + 1) * sizeof(unsigned long));
        hart_mask[id / sizeof(unsigned long) * 8] |= 1 << (id % (sizeof(unsigned long) * 8));
        catch_e(sbi_hart_start(id, KERNBASE, stack_top));
        catch_e(sbi_send_ipi(hart_mask));
      } else {
        info("%s (master) probed (stack top: %016lx)\n", node->nd_name, (unsigned long)KSTKTOP);
      }
      cpu_count++;
      break;
    }
  }

  if (!reg_found) {
    return -KER_DTB_ER;
  }

  return KER_SUCCESS;
}

struct device cpu_device = {
    .d_name = "cpu",
    .d_probe = cpu_probe,
    .d_probe_pri = HIGHEST,
};

unsigned long cpu_get_count(void) {
  return cpu_count;
}

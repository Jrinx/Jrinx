#include <endian.h>
#include <kern/drivers/device.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lib/hart.h>
#include <kern/lib/sbi.h>
#include <kern/mm/mem.h>
#include <layouts.h>
#include <lib/string.h>

unsigned long *cpus_stacktop = NULL;

static unsigned long cpus_count;

unsigned long cpus_get_count(void) {
  return cpus_count;
}

static long cpus_probe(const struct dev_node *node) {
  if (strcmp(node->nd_name, "cpus") != 0) {
    return KER_SUCCESS;
  }

  cpus_count = 0;
  struct dev_node *child;
  TAILQ_FOREACH (child, &node->nd_children_tailq, nd_link) {
    if (dt_node_has_dev_type(child, "cpu")) {
      cpus_count++;
    }
  }

  cpus_stacktop = alloc(sizeof(unsigned long) * cpus_count, sizeof(unsigned long));
  size_t hart_mask_size = (cpus_count - 1) / (sizeof(unsigned long) * 8) + 1;
  unsigned long hart_mask[hart_mask_size];
  memset(hart_mask, 0, hart_mask_size * sizeof(unsigned long));

  TAILQ_FOREACH (child, &node->nd_children_tailq, nd_link) {
    if (dt_node_has_dev_type(child, "cpu")) {
      int reg_found = 0;
      struct dev_node_prop *prop;
      TAILQ_FOREACH (prop, &child->nd_prop_tailq, pr_link) {
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
            cpus_stacktop[id] = stack_top;
            info("%s (slave)  probed (stack top: %016lx)\n", child->nd_name, stack_top);
            hart_mask[id / (sizeof(unsigned long) * 8)] |=
                1UL << (id % (sizeof(unsigned long) * 8));
            catch_e(sbi_hart_start(id, KERNBASE, 0));
          } else {
            cpus_stacktop[id] = KSTKTOP;
            info("%s (master) probed (stack top: %016lx)\n", child->nd_name,
                 (unsigned long)KSTKTOP);
          }
          break;
        }
      }

      if (!reg_found) {
        return -KER_DTB_ER;
      }
    }
  }
  catch_e(sbi_send_ipi(hart_mask));
  return KER_SUCCESS;
}

struct device cpus_device = {
    .d_probe = cpus_probe,
    .d_probe_pri = HIGHEST,
};

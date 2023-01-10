#include <endian.h>
#include <kern/drivers/device.h>
#include <kern/drivers/realtime.h>
#include <kern/drivers/rtc/goldfish.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lock/lock.h>
#include <kern/lock/spinlock.h>
#include <kern/mm/pmm.h>
#include <kern/mm/vmm.h>

struct goldfish {
  char *gf_name;
  uint64_t gf_addr;
  uint64_t gf_size;
  LIST_ENTRY(goldfish) gf_link;
};

static LIST_HEAD(, goldfish) goldfish_list;

static long goldfish_read_time(void *ctx, uint64_t *re) {
  struct goldfish *goldfish = ctx;
  uint64_t lo = *((volatile uint32_t *)(goldfish->gf_addr + GOLDFISH_TIME_LOW));
  uint64_t hi = *((volatile uint32_t *)(goldfish->gf_addr + GOLDFISH_TIME_HIGH));
  *re = (hi << 32) | lo;
  return KER_SUCCESS;
}

static long goldfish_probe(const struct dev_node *node) {
  struct dev_node_prop *prop;
  prop = dt_node_prop_extract(node, "compatible");
  if (prop == NULL || !dt_match_strlist(prop->pr_values, prop->pr_len, "google,goldfish-rtc")) {
    return KER_SUCCESS;
  }

  prop = dt_node_prop_extract(node, "reg");
  if (prop == NULL) {
    return -KER_DTB_ER;
  }

  uint64_t addr = from_be(*((uint64_t *)prop->pr_values));
  uint64_t size = from_be(*((uint64_t *)prop->pr_values + 1));
  struct goldfish *goldfish = alloc(sizeof(struct goldfish), sizeof(struct goldfish));
  goldfish->gf_name = node->nd_name;
  goldfish->gf_addr = addr;
  goldfish->gf_size = size;
  LIST_INSERT_HEAD(&goldfish_list, goldfish, gf_link);

  info("%s probed\n", node->nd_name);
  info("\tlocates at ");
  mem_print_range(addr, size, NULL);

  cb_decl(read_time_callback_t, goldfish_read_time_callback, goldfish_read_time, goldfish);
  rt_register_dev(node->nd_name, goldfish_read_time_callback);

  vmm_register_mmio(goldfish->gf_name, &goldfish->gf_addr, goldfish->gf_size);

  return KER_SUCCESS;
}

struct device goldfish_device = {
    .d_probe = goldfish_probe,
    .d_probe_pri = HIGHEST,
};

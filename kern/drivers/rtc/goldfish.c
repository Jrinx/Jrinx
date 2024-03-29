#include "goldfish.h"
#include <endian.h>
#include <kern/drivers/device.h>
#include <kern/drivers/realtime.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lock/lock.h>
#include <kern/lock/spinlock.h>
#include <kern/mm/kalloc.h>
#include <kern/mm/vmm.h>

struct goldfish {
  char *gf_name;
  uintptr_t gf_addr;
  uintmax_t gf_size;
};

static long goldfish_read_time(void *ctx, uint64_t *re) {
  struct goldfish *goldfish = ctx;
  uint64_t lo = *((volatile uint32_t *)(goldfish->gf_addr + GOLDFISH_TIME_LOW));
  uint64_t hi = *((volatile uint32_t *)(goldfish->gf_addr + GOLDFISH_TIME_HIGH));
  *re = (hi << 32) | lo;
  return KER_SUCCESS;
}

static int goldfish_pred(const struct dev_node *node) {
  struct dev_node_prop *prop = dt_node_prop_extract(node, "compatible");
  return prop != NULL && dt_match_strlist(prop->pr_values, prop->pr_len, "google,goldfish-rtc");
}

static long goldfish_probe(const struct dev_node *node) {
  struct dev_node_prop *prop;
  prop = dt_node_prop_extract(node, "reg");
  if (prop == NULL) {
    return -KER_DTB_ER;
  }

  uint64_t addr = from_be(*((uint64_t *)prop->pr_values));
  uint64_t size = from_be(*((uint64_t *)prop->pr_values + 1));
  struct goldfish *goldfish = kalloc(sizeof(struct goldfish));
  goldfish->gf_name = node->nd_name;
  goldfish->gf_addr = addr;
  goldfish->gf_size = size;

  info("%s probed\n", node->nd_name);
  struct fmt_mem_range mem_range = {.addr = addr, .size = size};
  info("\tlocates at %pM (size: %pB)\n", &mem_range, &size);

  cb_decl(read_time_callback_t, goldfish_read_time_callback, goldfish_read_time, goldfish);
  rt_register_dev(node->nd_name, goldfish_read_time_callback);

  vmm_register_mmio(goldfish->gf_name, &goldfish->gf_addr, goldfish->gf_size);

  return KER_SUCCESS;
}

static struct device goldfish_device = {
    .d_pred = goldfish_pred,
    .d_probe = goldfish_probe,
};

device_init(goldfish_device, low);

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
#include <layouts.h>

struct goldfish {
  char *gf_name;
  uint64_t gf_addr;
  uint64_t gf_size;
  LIST_ENTRY(goldfish) gf_link;
};

static LIST_HEAD(, goldfish) goldfish_list;

static long goldfish_setup_map(void *ctx) {
  struct goldfish *goldfish = ctx;
  uint64_t addr = goldfish->gf_addr;
  uint64_t size = goldfish->gf_size;
  vaddr_t va = {.val = addr + DEVOFFSET};
  paddr_t pa = {.val = addr};
  perm_t perm = {.bits = {.a = 1, .d = 1, .r = 1, .w = 1, .g = 1}};
  info("set up %s mapping at ", goldfish->gf_name);
  mem_print_range(addr + DEVOFFSET, size, NULL);
  for (; va.val < addr + DEVOFFSET + size; va.val += PGSIZE, pa.val += PGSIZE) {
    catch_e(pt_map(kern_pgdir, va, pa, perm));
  }
  goldfish->gf_addr += DEVOFFSET;
  return KER_SUCCESS;
}

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
  realtime_register_rtc(node->nd_name, goldfish_read_time_callback);

  cb_decl(mmio_setup_callback_t, goldfish_setup_callback, goldfish_setup_map, goldfish);
  vmm_register_mmio(goldfish_setup_callback);

  return KER_SUCCESS;
}

struct device goldfish_device = {
    .d_probe = goldfish_probe,
    .d_probe_pri = HIGHEST,
};

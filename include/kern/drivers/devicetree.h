#ifndef _KERN_DRIVERS_DEVICETREE_H_
#define _KERN_DRIVERS_DEVICETREE_H_

#include <stdint.h>
#include <sys/queue.h>

struct dev_rsvmem {
  unsigned long r_addr;
  unsigned long r_size;
  TAILQ_ENTRY(dev_rsvmem) r_link;
};

TAILQ_HEAD(dev_rsvmem_tailq, dev_rsvmem);

struct dev_node_prop {
  char *pr_name;
  uint32_t pr_len;
  uint8_t *pr_values;
  TAILQ_ENTRY(dev_node_prop) pr_link;
};

TAILQ_HEAD(dev_node_prop_tailq, dev_node_prop);

TAILQ_HEAD(dev_node_tailq, dev_node);

struct dev_node {
  char *nd_name;
  struct dev_node_prop_tailq nd_prop_tailq;
  struct dev_node_tailq nd_children_tailq;
  TAILQ_ENTRY(dev_node) nd_link;
};

struct dev_tree {
  struct dev_rsvmem_tailq dt_rsvmem_tailq;
  struct dev_node_tailq dt_node_tailq;
};

long dt_load(void *dtb_addr, struct dev_tree *dt) __attribute__((warn_unused_result));
void dt_find(struct dev_tree *dt, const char *name, struct dev_node **p_node);
void dt_print_tree(struct dev_tree *dt);

#endif

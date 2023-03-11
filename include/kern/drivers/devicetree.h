#ifndef _KERN_DRIVERS_DEVICETREE_H_
#define _KERN_DRIVERS_DEVICETREE_H_

#include <lib/hashmap.h>
#include <list.h>
#include <stdint.h>

struct dev_rsvmem {
  unsigned long r_addr;
  unsigned long r_size;
  struct linked_node r_link;
};

struct dev_node_prop {
  char *pr_name;
  uint32_t pr_len;
  uint8_t *pr_values;
  struct linked_node pr_link;
};

struct dev_node {
  char *nd_name;
  struct hashmap nd_prop_map;
  struct list_head nd_children_list;
  struct linked_node nd_link;
};

struct dev_tree {
  struct list_head dt_rsvmem_list;
  struct list_head dt_node_list;
};

typedef int (*dt_node_pred_t)(const struct dev_node *node);
typedef long (*dt_iter_callback_t)(const struct dev_node *node);

struct dev_node_prop *dt_node_prop_extract(const struct dev_node *node, const char *prop_name);
int dt_match_strlist(const uint8_t *prop_values, uint32_t prop_len, const char *target);
long dt_load(void *dtb_addr, struct dev_tree *dt) __attribute__((warn_unused_result));
long dt_iter(struct dev_tree *dt, dt_node_pred_t pred, dt_iter_callback_t callback)
    __attribute__((warn_unused_result));
void dt_print_tree(struct dev_tree *dt);
const char *dt_get_model(const struct dev_tree *dt);

#endif

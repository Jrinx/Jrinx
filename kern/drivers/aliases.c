#include <kern/drivers/device.h>
#include <kern/drivers/devicetree.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/mm/kalloc.h>
#include <lib/string.h>

struct aliases_item {
  const char *ai_key;
  const char *ai_val;
  struct linked_node ai_link;
};

static const void *aliases_key_of(const struct linked_node *node) {
  const struct aliases_item *item = CONTAINER_OF(node, struct aliases_item, ai_link);
  return item->ai_key;
}

static struct hlist_head aliases_map_array[8];
static struct hashmap aliases_map = {
    .h_array = aliases_map_array,
    .h_num = 0,
    .h_cap = 8,
    .h_code = hash_code_str,
    .h_equals = hash_eq_str,
    .h_key = aliases_key_of,
};

const char *aliases_get(const char *key) {
  struct linked_node *node = hashmap_get(&aliases_map, key);
  if (node == NULL) {
    return NULL;
  }
  struct aliases_item *item = CONTAINER_OF(node, struct aliases_item, ai_link);
  return item->ai_val;
}

static int aliases_pred(const struct dev_node *node) {
  return strcmp(node->nd_name, "aliases") == 0;
}

static long aliases_probe(const struct dev_node *node) {
  info("%s probed, props listed:\n", node->nd_name);
  struct dev_node_prop *prop;
  HASHMAP_ITER (&node->nd_prop_map, prop, pr_link) {
    struct aliases_item *item = kalloc(sizeof(struct aliases_item));
    item->ai_key = prop->pr_name;
    item->ai_val = (char *)prop->pr_values;
    info("\t%s: %s\n", item->ai_key, item->ai_val);
    hashmap_put(&aliases_map, &item->ai_link);
  }
  return KER_SUCCESS;
}

static struct device aliases_device = {
    .d_pred = aliases_pred,
    .d_probe = aliases_probe,
};

device_init(aliases_device, lowest);

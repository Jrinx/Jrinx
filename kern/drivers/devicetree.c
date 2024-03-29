#include <endian.h>
#include <kern/drivers/devicetree.h>
#include <kern/drivers/dtb.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/mm/kalloc.h>
#include <lib/printfmt.h>
#include <lib/string.h>
#include <stddef.h>

static inline long fdt_from(void *dtb_addr, struct fdt_header **pfhdr) {
  struct fdt_header *fhdr = dtb_addr;
  if (from_be(fhdr->h_magic) != DTB_FDT_MAGIC) {
    return -KER_DTB_ER;
  }
  if (from_be(fhdr->h_version) != DTB_SUPPORTED_VER) {
    return -KER_DTB_ER;
  }
  *pfhdr = fhdr;
  return KER_SUCCESS;
}

static const void *prop_key_of(const struct linked_node *node) {
  const struct dev_node_prop *prop = CONTAINER_OF(node, struct dev_node_prop, pr_link);
  return prop->pr_name;
}

static long dt_node_load(void *dtb_addr, size_t pos, size_t *nxt_pos,
                         struct list_head *child_list) {
  struct fdt_header *fhdr = dtb_addr;
  uint32_t dtb_struct_off = from_be(fhdr->h_off_dt_struct);
  uint32_t dtb_strings_off = from_be(fhdr->h_off_dt_strings);
  uint32_t struct_token;

  assert(pos * sizeof(uint32_t) < from_be(fhdr->h_size_dt_struct));

  uint32_t *dtb_struct_table = dtb_addr + dtb_struct_off;

  while ((struct_token = from_be(dtb_struct_table[pos])) == FDT_NOP) {
    pos++;
  }

  if (struct_token != FDT_BEGIN_NODE) {
    return -KER_DTB_ER;
  }

  pos++;

  struct dev_node *node = kalloc(sizeof(struct dev_node));
  const char *node_name = (char *)(dtb_addr + dtb_struct_off + pos * sizeof(uint32_t));
  size_t node_name_len = strlen(node_name);
  node->nd_name = kalloc(sizeof(char) * (node_name_len + 1));
  strcpy(node->nd_name, node_name);
  HASHMAP_ALLOC(&node->nd_prop_map, kalloc(sizeof(struct hlist_head) * 32), 32, str,
                prop_key_of);
  hashmap_init(&node->nd_prop_map);
  list_init(&node->nd_children_list);
  list_insert_tail(child_list, &node->nd_link);

  pos = (pos * sizeof(uint32_t) + node_name_len) / sizeof(uint32_t) + 1;

  while ((struct_token = from_be(dtb_struct_table[pos])) == FDT_NOP) {
    pos++;
  }

  while (struct_token == FDT_PROP) {
    uint32_t prop_len = from_be(dtb_struct_table[pos + 1]);
    uint32_t prop_nameoff = from_be(dtb_struct_table[pos + 2]);

    pos += 3;

    struct dev_node_prop *prop = kalloc(sizeof(struct dev_node_prop));
    const char *prop_name = (char *)(dtb_addr + dtb_strings_off + prop_nameoff);
    size_t prop_name_len = strlen(prop_name);
    prop->pr_name = kalloc(sizeof(char) * (prop_name_len + 1));
    strcpy(prop->pr_name, prop_name);
    prop->pr_len = prop_len;
    if (prop_len != 0) {
      prop->pr_values = kalloc(sizeof(uint8_t) * prop_len);
      memcpy(prop->pr_values, dtb_addr + dtb_struct_off + pos * sizeof(uint32_t), prop_len);
    } else {
      prop->pr_values = NULL;
    }
    hashmap_put(&node->nd_prop_map, &prop->pr_link);

    pos = (pos * sizeof(uint32_t) + prop_len - 1) / sizeof(uint32_t) + 1;

    while ((struct_token = from_be(dtb_struct_table[pos])) == FDT_NOP) {
      pos++;
    }
  }

  do {
    if (struct_token == FDT_BEGIN_NODE) {
      catch_e(dt_node_load(dtb_addr, pos, &pos, &node->nd_children_list));
    } else if (struct_token == FDT_NOP) {
      pos++;
    } else {
      break;
    }
  } while ((struct_token = from_be(dtb_struct_table[pos])) != FDT_END_NODE);

  pos++;

  if (nxt_pos) {
    *nxt_pos = pos;
  }

  return KER_SUCCESS;
}

long dt_load(void *dtb_addr, struct dev_tree *dt) {
  struct fdt_header *fhdr;
  catch_e(fdt_from(dtb_addr, &fhdr));

  uint32_t dtb_memrsv_off = from_be(fhdr->h_off_mem_rsvmap);

  list_init(&dt->dt_rsvmem_list);
  list_init(&dt->dt_node_list);

  for (size_t pos = dtb_memrsv_off;;) {
    struct fdt_reserve_entry *rsvent = (struct fdt_reserve_entry *)(dtb_addr + pos);
    pos += sizeof(struct fdt_reserve_entry);
    if (rsvent->e_addr == 0 && rsvent->e_size == 0) {
      break;
    }
    struct dev_rsvmem *dev_rsvmem_ent = kalloc(sizeof(struct dev_rsvmem));
    dev_rsvmem_ent->r_addr = rsvent->e_addr;
    dev_rsvmem_ent->r_size = rsvent->e_size;
    list_insert_tail(&dt->dt_rsvmem_list, &dev_rsvmem_ent->r_link);
  }

  catch_e(dt_node_load(dtb_addr, 0, NULL, &dt->dt_node_list));

  return KER_SUCCESS;
}

struct dev_node_prop *dt_node_prop_extract(const struct dev_node *node, const char *prop_name) {
  struct linked_node *linked_node = hashmap_get(&node->nd_prop_map, prop_name);
  if (linked_node == NULL) {
    return NULL;
  }
  struct dev_node_prop *prop = CONTAINER_OF(linked_node, struct dev_node_prop, pr_link);
  return prop;
}

int dt_match_strlist(const uint8_t *prop_values, uint32_t prop_len, const char *target) {
  size_t target_bytes_len = strlen(target) + 1;
  uint8_t *target_bytes = (uint8_t *)target;
  if (prop_len < target_bytes_len) {
    return 0;
  }

  size_t i;
  size_t j;
  for (i = 0, j = 0; i < target_bytes_len && j < prop_len; j++) {
    if (target_bytes[i] != prop_values[j]) {
      i = 0;
    } else {
      i++;
    }
  }

  return i == target_bytes_len ? j : 0;
}

static long dt_iter_node(struct dev_node *node, dt_node_pred_t pred,
                         dt_iter_callback_t callback) {
  struct dev_node *child;
  LINKED_NODE_ITER (node->nd_children_list.l_first, child, nd_link) {
    if (pred(child)) {
      catch_e(callback(child));
    }
    catch_e(dt_iter_node(child, pred, callback));
  }
  return KER_SUCCESS;
}

long dt_iter(struct dev_tree *dt, dt_node_pred_t pred, dt_iter_callback_t callback) {
  struct dev_node *node;
  LINKED_NODE_ITER (dt->dt_node_list.l_first, node, nd_link) {
    if (pred(node)) {
      catch_e(callback(node));
    }
    catch_e(dt_iter_node(node, pred, callback));
  }
  return KER_SUCCESS;
}

static inline void dt_print_indention(unsigned long layer) {
  const char indent_char = ' ';
  const unsigned int char_rep = 2;
  for (size_t i = 0; i < layer; i++) {
    for (size_t j = 0; j < char_rep; j++) {
      printk("%c", indent_char);
    }
  }
}

static inline void dt_print_node_header(const char *name) {
  if (unlikely(name[0])) {
    printk("%s {\n", name);
  } else {
    printk("{\n");
  }
}

static void dt_print_node(struct dev_node *node, unsigned long layer) {
#define dt_printl(fmt, ...)                                                                    \
  ({                                                                                           \
    dt_print_indention(layer);                                                                 \
    printk(fmt, ##__VA_ARGS__);                                                                \
  })

  struct dev_node_prop *prop;
  HASHMAP_ITER (&node->nd_prop_map, prop, pr_link) {
    dt_printl("%s: ", prop->pr_name);
    for (size_t i = 0; i < prop->pr_len; i++) {
      printk("%02x", prop->pr_values[i]);
      if (i < prop->pr_len - 1) {
        printk(" ");
      }
    }
    printk("\n");
  }

  struct dev_node *child;
  LINKED_NODE_ITER (node->nd_children_list.l_first, child, nd_link) {
    dt_printl("%s {\n", child->nd_name);
    dt_print_node(child, layer + 1);
    dt_printl("}\n");
  }
#undef dt_printl
}

void dt_print_tree(struct dev_tree *dt) {
  printk("/dts-v1/;\n\n");

  struct dev_rsvmem *rsvmem;
  LINKED_NODE_ITER (dt->dt_rsvmem_list.l_first, rsvmem, r_link) {
    printk("/memreserve/\t0x%16lx 0x%16lx;\n", rsvmem->r_addr, rsvmem->r_size);
  }

  struct dev_node *node;
  LINKED_NODE_ITER (dt->dt_node_list.l_first, node, nd_link) {
    dt_print_indention(0);
    dt_print_node_header(node->nd_name);
    dt_print_node(node, 1);
    dt_print_indention(0);
    printk("}\n");
  }
}

const char *dt_get_model(const struct dev_tree *dt) {
  struct dev_node *node;
  LINKED_NODE_ITER (dt->dt_node_list.l_first, node, nd_link) {
    struct dev_node_prop *prop;
    prop = dt_node_prop_extract(node, "model");
    if (prop != NULL) {
      return (char *)prop->pr_values;
    }
  }
  return NULL;
}

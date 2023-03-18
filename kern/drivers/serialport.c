#include <kern/drivers/serialport.h>
#include <kern/lib/debug.h>
#include <kern/mm/kalloc.h>
#include <kern/mm/vmm.h>
#include <lib/hashmap.h>
#include <lib/string.h>
#include <stddef.h>
#include <stdint.h>

struct serial_dev {
  char *sr_name;
  putc_callback_t sr_putc_callback;
  getc_callback_t sr_getc_callback;
  struct linked_node sr_link;
};

static struct serial_dev *selected_output_dev;
static struct serial_dev *selected_input_dev;

static const void *serial_key_of(const struct linked_node *node) {
  const struct serial_dev *dev = CONTAINER_OF(node, struct serial_dev, sr_link);
  return dev->sr_name;
}

static struct hlist_head serial_map_array[16];
static struct hashmap serial_map = {
    .h_array = serial_map_array,
    .h_num = 0,
    .h_cap = 16,
    .h_code = hash_code_str,
    .h_equals = hash_eq_str,
    .h_key = serial_key_of,
};

void serial_register_dev(char *name, putc_callback_t putc_callback,
                         getc_callback_t getc_callback) {
  struct serial_dev *dev = kalloc(sizeof(struct serial_dev));
  dev->sr_name = name;
  dev->sr_putc_callback = putc_callback;
  dev->sr_getc_callback = getc_callback;
  hashmap_put(&serial_map, &dev->sr_link);
  if (selected_input_dev == NULL) {
    selected_input_dev = dev;
  }
  if (selected_output_dev == NULL) {
    selected_output_dev = dev;
  }
}

int serial_select_out_dev(const char *name) {
  struct linked_node *node = hashmap_get(&serial_map, name);
  if (node == NULL) {
    return 0;
  }
  struct serial_dev *dev = CONTAINER_OF(node, struct serial_dev, sr_link);
  info("select %s as serial output device\n", name);
  selected_output_dev = dev;
  return 1;
}

int serial_select_in_dev(const char *name) {
  struct linked_node *node = hashmap_get(&serial_map, name);
  if (node == NULL) {
    return 0;
  }
  struct serial_dev *dev = CONTAINER_OF(node, struct serial_dev, sr_link);
  info("select %s as serial input device\n", name);
  selected_input_dev = dev;
  return 1;
}

int serial_getc(uint8_t *c) {
  return cb_invoke(selected_input_dev->sr_getc_callback)(c);
}

int serial_putc(uint8_t c) {
  if (!cb_invoke(selected_output_dev->sr_putc_callback)(c)) {
    return 0;
  }
  if (c == '\n') {
    serial_blocked_putc('\r');
  }
  return 1;
}

uint8_t serial_blocked_getc(void) {
  uint8_t c;
  while (!serial_getc(&c)) {
  }
  return c;
}

void serial_blocked_putc(uint8_t c) {
  while (!serial_putc(c)) {
  }
}

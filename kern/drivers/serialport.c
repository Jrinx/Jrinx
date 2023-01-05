#include <kern/drivers/serialport.h>
#include <kern/lib/debug.h>
#include <kern/mm/pmm.h>
#include <kern/mm/vmm.h>
#include <lib/string.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/queue.h>

struct serial_dev {
  char *sr_name;
  putc_func_t sr_putc_func;
  getc_func_t sr_getc_func;
  LIST_ENTRY(serial_dev) sr_link;
};

static struct serial_dev *selected_output_dev;
static struct serial_dev *selected_input_dev;
static LIST_HEAD(, serial_dev) serial_dev_list;

void serial_register_dev(const char *name, putc_func_t putc_func, getc_func_t getc_func) {
  struct serial_dev *dev = alloc(sizeof(struct serial_dev), sizeof(struct serial_dev));
  size_t name_len = strlen(name);
  dev->sr_name = alloc(sizeof(char) * (name_len + 1), sizeof(char));
  strcpy(dev->sr_name, name);
  dev->sr_putc_func = putc_func;
  dev->sr_getc_func = getc_func;
  LIST_INSERT_HEAD(&serial_dev_list, dev, sr_link);
  selected_output_dev = dev;
  selected_input_dev = dev;
}

int serial_select_out_dev(const char *name) {
  int found = 0;
  struct serial_dev *dev;
  LIST_FOREACH (dev, &serial_dev_list, sr_link) {
    if (strcmp(dev->sr_name, name) == 0) {
      info("select %s as serial output device\n", name);
      found = 1;
      selected_output_dev = dev;
      break;
    }
  }
  return found;
}

int serial_select_in_dev(const char *name) {
  int found = 0;
  struct serial_dev *dev;
  LIST_FOREACH (dev, &serial_dev_list, sr_link) {
    if (strcmp(dev->sr_name, name) == 0) {
      info("select %s as serial input device\n", name);
      found = 1;
      selected_input_dev = dev;
      break;
    }
  }
  return found;
}

int serial_getc(uint8_t *c) {
  return selected_input_dev->sr_getc_func(c);
}

int serial_putc(uint8_t c) {
  return selected_output_dev->sr_putc_func(c);
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

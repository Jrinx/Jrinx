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
  putc_callback_t sr_putc_callback;
  getc_callback_t sr_getc_callback;
  LIST_ENTRY(serial_dev) sr_link;
};

static struct serial_dev *selected_output_dev;
static struct serial_dev *selected_input_dev;
static LIST_HEAD(, serial_dev) serial_dev_list;

void serial_register_dev(char *name, putc_callback_t putc_callback,
                         getc_callback_t getc_callback) {
  struct serial_dev *dev = alloc(sizeof(struct serial_dev), sizeof(struct serial_dev));
  dev->sr_name = name;
  dev->sr_putc_callback = putc_callback;
  dev->sr_getc_callback = getc_callback;
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
  return cb_invoke(selected_input_dev->sr_getc_callback)(c);
}

int serial_putc(uint8_t c) {
  return cb_invoke(selected_output_dev->sr_putc_callback)(c);
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

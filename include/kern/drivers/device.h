#ifndef _KERN_DRIVERS_DEVICE_H_
#define _KERN_DRIVERS_DEVICE_H_

#include <kern/drivers/devicetree.h>

typedef long (*probe_t)(const struct dev_node *node);

enum probe_pri_t {
  HIGHEST,
  MEDIUM,
  LOWEST,
};

struct device {
  char *d_name;
  probe_t d_probe;
  enum probe_pri_t d_probe_pri;
  TAILQ_ENTRY(device) d_link;
};

TAILQ_HEAD(dev_queue_t, device);

long dev_register(struct device *dev);
long device_init(void);
long device_probe(struct dev_tree *dt);

#endif

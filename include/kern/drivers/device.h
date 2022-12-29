#ifndef _KERN_DRIVERS_DEVICE_H_
#define _KERN_DRIVERS_DEVICE_H_

#include <kern/drivers/devicetree.h>

enum probe_pri_t {
  HIGHEST,
  MEDIUM,
  LOWEST,
};

struct device {
  dt_iter_callback_t d_probe;
  enum probe_pri_t d_probe_pri;
  TAILQ_ENTRY(device) d_link;
};

TAILQ_HEAD(dev_queue_t, device);

long dev_register(struct device *dev) __attribute__((warn_unused_result));
long device_init(void) __attribute__((warn_unused_result));
long device_probe(struct dev_tree *dt) __attribute__((warn_unused_result));

#endif

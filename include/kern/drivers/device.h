#ifndef _KERN_DRIVERS_DEVICE_H_
#define _KERN_DRIVERS_DEVICE_H_

#include <kern/drivers/devicetree.h>

enum probe_pri_t {
  HIGHEST,
  HIGH,
  MEDIUM,
  LOW,
  LOWEST,
};

struct device {
  dt_node_pred_t d_pred;
  dt_iter_callback_t d_probe;
  enum probe_pri_t d_probe_pri;
};

long device_probe(struct dev_tree *dt) __attribute__((warn_unused_result));

#endif

#ifndef _KERN_DRIVERS_DEVICE_H_
#define _KERN_DRIVERS_DEVICE_H_

#include <attr.h>
#include <kern/drivers/devicetree.h>

struct device {
  dt_node_pred_t d_pred;
  dt_iter_callback_t d_probe;
};

long device_probe(struct dev_tree *dt) __warn_unused_result;

#define device_init(dev, priority)                                                             \
  struct device *dev##_init __section(".ksec.dev_init_" #priority "." #dev) __used = &dev

#endif

#include <kern/drivers/device.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/mm/pmm.h>

long device_probe(struct dev_tree *dt) {
  extern struct device *kern_dev_init_begin[];
  extern struct device *kern_dev_init_end[];
  for (struct device **ptr = kern_dev_init_begin; ptr < kern_dev_init_end; ptr++) {
    struct device *dev = *ptr;
    catch_e(dt_iter(dt, dev->d_pred, dev->d_probe));
  }
  return KER_SUCCESS;
}

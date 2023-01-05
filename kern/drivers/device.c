#include <kern/drivers/chosen.h>
#include <kern/drivers/cpus.h>
#include <kern/drivers/device.h>
#include <kern/drivers/irq/plic.h>
#include <kern/drivers/mems.h>
#include <kern/drivers/serial/uart16550a.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/mm/pmm.h>

static struct dev_queue_t dev_queue;

long dev_register(struct device *dev) {
  if (TAILQ_NEXT(dev, d_link) != NULL) {
    TAILQ_REMOVE(&dev_queue, dev, d_link);
  }
  switch (dev->d_probe_pri) {
  case HIGHEST:
    TAILQ_INSERT_HEAD(&dev_queue, dev, d_link);
    break;
  case LOWEST:
    TAILQ_INSERT_TAIL(&dev_queue, dev, d_link);
    break;
  case MEDIUM:
    struct device *d;
    if (TAILQ_EMPTY(&dev_queue)) {
      TAILQ_INSERT_HEAD(&dev_queue, dev, d_link);
    } else {
      TAILQ_FOREACH (d, &dev_queue, d_link) {
        if (TAILQ_NEXT(d, d_link) == NULL || TAILQ_NEXT(d, d_link)->d_probe_pri != HIGHEST) {
          TAILQ_INSERT_AFTER(&dev_queue, d, dev, d_link);
          break;
        }
      }
    }
    break;
  default:
    return -KER_DEV_ER;
  }
  return KER_SUCCESS;
}

long device_init(void) {
  TAILQ_INIT(&dev_queue);
  catch_e(dev_register(&memory_device));
  catch_e(dev_register(&cpus_device));
  catch_e(dev_register(&plic_device));
  catch_e(dev_register(&uart16550a_device));
  catch_e(dev_register(&chosen_device));
  return KER_SUCCESS;
}

long device_probe(struct dev_tree *dt) {
  struct device *dev;
  TAILQ_FOREACH (dev, &dev_queue, d_link) {
    catch_e(dt_iter(dt, dev->d_probe));
  }
  return KER_SUCCESS;
}

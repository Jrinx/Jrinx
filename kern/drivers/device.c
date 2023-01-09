#include <kern/drivers/chosen.h>
#include <kern/drivers/cpus.h>
#include <kern/drivers/device.h>
#include <kern/drivers/irq/plic.h>
#include <kern/drivers/mems.h>
#include <kern/drivers/rtc/goldfish.h>
#include <kern/drivers/serial/uart16550a.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/mm/pmm.h>

static struct dev_queue_t dev_queue;

long dev_register(struct device *dev) {
  TAILQ_INSERT_TAIL(&dev_queue, dev, d_link);
  return KER_SUCCESS;
}

long device_init(void) {
  TAILQ_INIT(&dev_queue);
  catch_e(dev_register(&memory_device));
  catch_e(dev_register(&cpus_device));
  catch_e(dev_register(&plic_device));
  catch_e(dev_register(&uart16550a_device));
  catch_e(dev_register(&chosen_device));
  catch_e(dev_register(&goldfish_device));
  return KER_SUCCESS;
}

long device_probe(struct dev_tree *dt) {
  for (unsigned int pri = HIGHEST; pri <= LOWEST; pri++) {
    struct device *dev;
    TAILQ_FOREACH (dev, &dev_queue, d_link) {
      if (dev->d_probe_pri == pri) {
        catch_e(dt_iter(dt, dev->d_probe));
      }
    }
  }
  return KER_SUCCESS;
}

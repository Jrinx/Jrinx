#include <kern/drivers/chosen.h>
#include <kern/drivers/cpus.h>
#include <kern/drivers/device.h>
#include <kern/drivers/irq/plic.h>
#include <kern/drivers/mems.h>
#include <kern/drivers/rtc/goldfish.h>
#include <kern/drivers/serial/sifiveuart0.h>
#include <kern/drivers/serial/uart16550a.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/mm/pmm.h>

static struct device *dev_queue[] = {
    &memory_device,     &cpus_device,   &plic_device,     &sifiveuart0_device,
    &uart16550a_device, &chosen_device, &goldfish_device, NULL,
};

long device_probe(struct dev_tree *dt) {
  for (unsigned int pri = HIGHEST; pri <= LOWEST; pri++) {
    for (size_t i = 0; dev_queue[i] != NULL; i++) {
      struct device *dev = dev_queue[i];
      if (dev->d_probe_pri == pri) {
        catch_e(dt_iter(dt, dev->d_pred, dev->d_probe));
      }
    }
  }
  return KER_SUCCESS;
}

#ifndef _KERN_DRIVERS_IRQ_PLIC_H_
#define _KERN_DRIVERS_IRQ_PLIC_H_

#include <kern/drivers/device.h>

#define PLIC_EXTERNAL_INT_CTX 1U

#define PLIC_SOURCE_MIN 1U
#define PLIC_SOURCE_MAX 53U
#define PLIC_CONTEXT_MIN 0U
#define PLIC_CONTEXT_MAX 4U
#define PLIC_PRIO_MIN 0U
#define PLIC_PRIO_MAX 7U

enum plic_context_mode_t {
  M_MODE,
  S_MODE,
};

extern struct device plic_device;

#endif

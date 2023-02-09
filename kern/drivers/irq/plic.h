#ifndef PLIC_H
#define PLIC_H

#include <kern/drivers/device.h>
#include <layouts.h>

#if defined(CONFIG_BOARD_VIRT)
#define PLIC_EXTERNAL_INT_CTX (SYSCORE * 2 + 1)
#elif defined(CONFIG_BOARD_SIFIVE_U)
#define PLIC_EXTERNAL_INT_CTX (SYSCORE * 2)
#endif

#define PLIC_SOURCE_MIN 1U
#define PLIC_SOURCE_MAX 1023U
#define PLIC_PRIO_MIN 0U
#define PLIC_PRIO_MAX 7U

#endif

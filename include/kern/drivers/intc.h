#ifndef _KERN_DRIVERS_INTC_H_
#define _KERN_DRIVERS_INTC_H_

#include <callback.h>
#include <stdint.h>

typedef long (*trap_handler_t)(void *ctx, unsigned long trap_num);
typedef cb_typedef(trap_handler_t) trap_callback_t;
typedef long (*irq_register_func_t)(void *ctx, unsigned long int_num, trap_callback_t callback);
typedef cb_typedef(irq_register_func_t) irq_register_callback_t;

void intc_get_handler(unsigned long trap_num, trap_callback_t *callback);
long intc_register_handler(void *_, unsigned long trap_num, trap_callback_t callback);

void intc_get_irq_reg(uint32_t phandle_num, irq_register_callback_t *callback);
void intc_register_irq_reg(uint32_t phandle_num, irq_register_callback_t callback);

#endif

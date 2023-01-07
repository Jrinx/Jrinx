#ifndef _KERN_DRIVERS_INTC_H_
#define _KERN_DRIVERS_INTC_H_

#include <currying.h>
#include <stdint.h>

typedef long (*trap_handler_t)(void *ctx, unsigned long trap_num);
typedef cb_typedef(trap_handler_t) trap_callback_t;
typedef long (*irq_register_func_t)(void *ctx, unsigned long int_num, trap_callback_t callback);
typedef cb_typedef(irq_register_func_t) irq_register_callback_t;

const trap_callback_t *intc_get_exc_vec(void);
long intc_register_handler(void *_, unsigned long trap_num, trap_callback_t callback);

void intc_get_register_func(uint32_t phandle_num, irq_register_callback_t *callback);
void intc_set_register_func(uint32_t phandle_num, irq_register_callback_t callback);

#endif

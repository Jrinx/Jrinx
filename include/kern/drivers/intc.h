#ifndef _KERN_DRIVERS_INTC_H_
#define _KERN_DRIVERS_INTC_H_

#include <stdint.h>

typedef long (*trap_handler_t)(void *ctx, unsigned long trap_num);
typedef long (*irq_phandle_t)(void *ctx, unsigned long int_num, trap_handler_t handler,
                              void *subctx);

const trap_handler_t *intc_get_exc_vec(void);
long intc_register_handler(void *_, unsigned long trap_num, trap_handler_t handler,
                           void *subctx);

irq_phandle_t intc_get_phandle(uint32_t phandle_num, void **ctx);
void intc_set_phandle(uint32_t phandle_num, irq_phandle_t func, void *ctx);

#endif

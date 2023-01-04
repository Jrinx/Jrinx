#ifndef _KERN_DRIVERS_INTC_H_
#define _KERN_DRIVERS_INTC_H_

#include <stdint.h>

typedef long (*trap_handler_t)(unsigned long trap_num);
typedef long (*irq_phandle_t)(unsigned long int_num, trap_handler_t handler);

const trap_handler_t *intc_get_int_vec(void);
const trap_handler_t *intc_get_exc_vec(void);
void intc_register_handler(unsigned long trap_num, trap_handler_t handler);

irq_phandle_t intc_get_phandle(uint8_t phandle_num);
void intc_set_phandle(uint8_t phandle_num, irq_phandle_t func);

#endif

#ifndef _KERN_DRIVERS_SERIALPORT_H_
#define _KERN_DRIVERS_SERIALPORT_H_

#include <stdint.h>

typedef int (*getc_func_t)(uint8_t *c);
typedef int (*putc_func_t)(uint8_t c);

void serial_register_dev(const char *name, putc_func_t putc_func, getc_func_t getc_func);
int serial_select_out_dev(const char *name);
int serial_select_in_dev(const char *name);
int serial_getc(uint8_t *c);
int serial_putc(uint8_t c);
uint8_t serial_blocked_getc(void);
void serial_blocked_putc(uint8_t c);

#endif

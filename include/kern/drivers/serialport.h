#ifndef _KERN_DRIVERS_SERIALPORT_H_
#define _KERN_DRIVERS_SERIALPORT_H_

#include <callback.h>
#include <stdint.h>

typedef int (*getc_func_t)(void *ctx, uint8_t *c);
typedef int (*putc_func_t)(void *ctx, uint8_t c);
typedef void (*flush_func_t)(void *ctx);
typedef cb_typedef(getc_func_t) getc_callback_t;
typedef cb_typedef(putc_func_t) putc_callback_t;
typedef cb_typedef(flush_func_t) flush_callback_t;

void serial_register_dev(char *name, flush_callback_t flush_callback,
                         putc_callback_t putc_callback, getc_callback_t getc_callback);
int serial_select_out_dev(const char *name);
int serial_select_in_dev(const char *name);
void serial_in_dev_lock_acquire(void);
void serial_in_dev_lock_release(void);
void serial_out_dev_lock_acquire(void);
void serial_out_dev_lock_release(void);
int serial_getc(uint8_t *c);
int serial_putc(uint8_t c);
uint8_t serial_blocked_getc(void);
void serial_blocked_putc(uint8_t c);
void serial_flush(void);

#endif

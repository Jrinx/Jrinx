#ifndef _USER_SYS_SYSCALLS_H_
#define _USER_SYS_SYSCALLS_H_

#include <stddef.h>

void sys_cons_write_char(int ch);
int sys_cons_read_char(void);
void sys_cons_write_buf(const char *buf, size_t len);
void sys_yield(void);

#endif

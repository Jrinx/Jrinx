#ifndef _USER_LIB_SYSCALLS_H_
#define _USER_LIB_SYSCALLS_H_

long sys_write_cons(int ch);
int sys_read_cons(void);
void sys_yield(void);

#endif

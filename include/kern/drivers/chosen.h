#ifndef _KERN_DRIVERS_CHOSEN_H_
#define _KERN_DRIVERS_CHOSEN_H_

extern struct device chosen_device;
const char *chosen_get_bootargs(void);
const char *chosen_get_stdout_dev_name(void);
const char *chosen_get_stdin_dev_name(void);
long chosen_select_dev(void) __attribute__((warn_unused_result));

#endif

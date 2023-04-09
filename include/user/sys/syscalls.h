#ifndef _USER_SYS_SYSCALLS_H_
#define _USER_SYS_SYSCALLS_H_

#include <stddef.h>
#include <types.h>

void sys_cons_write_char(int ch);
int sys_cons_read_char(void);
void sys_cons_write_buf(const char *buf, size_t len);
void sys_halt(void) __attribute__((noreturn));

RETURN_CODE_TYPE sys_get_partition_status(PARTITION_STATUS_TYPE *partition_status);
RETURN_CODE_TYPE sys_set_partition_mode(OPERATING_MODE_TYPE operating_mode);
RETURN_CODE_TYPE sys_get_process_id(PROCESS_NAME_TYPE process_name,
                                    PROCESS_ID_TYPE *process_id);
RETURN_CODE_TYPE sys_get_process_status(PROCESS_ID_TYPE process_id,
                                        PROCESS_STATUS_TYPE *process_status);
RETURN_CODE_TYPE sys_create_process(PROCESS_ATTRIBUTE_TYPE *attributes,
                                    PROCESS_ID_TYPE *process_id);
RETURN_CODE_TYPE sys_set_priority(PROCESS_ID_TYPE process_id, PRIORITY_TYPE priority);
RETURN_CODE_TYPE sys_suspend_self(SYSTEM_TIME_TYPE time_out);
RETURN_CODE_TYPE sys_suspend(PROCESS_ID_TYPE process_id);
RETURN_CODE_TYPE sys_resume(PROCESS_ID_TYPE process_id);
void sys_stop_self(void) __attribute__((noreturn));
RETURN_CODE_TYPE sys_stop(PROCESS_ID_TYPE process_id);
RETURN_CODE_TYPE sys_start(PROCESS_ID_TYPE process_id);
RETURN_CODE_TYPE sys_delayed_start(PROCESS_ID_TYPE process_id, SYSTEM_TIME_TYPE delay_time);
RETURN_CODE_TYPE sys_lock_preemption(LOCK_LEVEL_TYPE *lock_level);
RETURN_CODE_TYPE sys_unlock_preemption(LOCK_LEVEL_TYPE *lock_level);
RETURN_CODE_TYPE sys_get_my_id(PROCESS_ID_TYPE *process_id);
RETURN_CODE_TYPE sys_initialize_process_core_affinity(PROCESS_ID_TYPE process_id,
                                                      PROCESSOR_CORE_ID_TYPE processor_core_id);
RETURN_CODE_TYPE sys_get_my_processor_core_id(PROCESSOR_CORE_ID_TYPE *processor_core_id);
RETURN_CODE_TYPE sys_get_my_index(PROCESS_INDEX_TYPE *process_index);
RETURN_CODE_TYPE sys_timed_wait(SYSTEM_TIME_TYPE delay_time);
RETURN_CODE_TYPE sys_periodic_wait(void);
RETURN_CODE_TYPE sys_get_time(SYSTEM_TIME_TYPE *system_time);
RETURN_CODE_TYPE sys_replenish(SYSTEM_TIME_TYPE budget_time);

#endif

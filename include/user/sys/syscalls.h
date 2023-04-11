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
RETURN_CODE_TYPE sys_create_sampling_port(SAMPLING_PORT_NAME_TYPE sampling_port_name,
                                          MESSAGE_SIZE_TYPE max_message_size,
                                          PORT_DIRECTION_TYPE port_direction,
                                          SYSTEM_TIME_TYPE refresh_period,
                                          SAMPLING_PORT_ID_TYPE *sampling_port_id);
RETURN_CODE_TYPE sys_write_sampling_message(SAMPLING_PORT_ID_TYPE sampling_port_id,
                                            MESSAGE_ADDR_TYPE message_addr,
                                            MESSAGE_SIZE_TYPE length);
RETURN_CODE_TYPE sys_read_sampling_message(SAMPLING_PORT_ID_TYPE sampling_port_id,
                                           MESSAGE_ADDR_TYPE message_addr,
                                           MESSAGE_SIZE_TYPE *length, VALIDITY_TYPE *validity);
RETURN_CODE_TYPE sys_get_sampling_port_id(SAMPLING_PORT_NAME_TYPE sampling_port_name,
                                          SAMPLING_PORT_ID_TYPE *sampling_port_id);
RETURN_CODE_TYPE sys_get_sampling_port_status(SAMPLING_PORT_ID_TYPE sampling_port_id,
                                              SAMPLING_PORT_STATUS_TYPE *sampling_port_status);
RETURN_CODE_TYPE sys_create_queuing_port(QUEUING_PORT_NAME_TYPE queuing_port_name,
                                         MESSAGE_SIZE_TYPE max_message_size,
                                         MESSAGE_RANGE_TYPE max_nb_message,
                                         PORT_DIRECTION_TYPE port_direction,
                                         QUEUING_DISCIPLINE_TYPE queuing_discipline,
                                         QUEUING_PORT_ID_TYPE *queuing_port_id);
RETURN_CODE_TYPE sys_send_queuing_message(QUEUING_PORT_ID_TYPE queuing_port_id,
                                          MESSAGE_ADDR_TYPE message_addr,
                                          MESSAGE_SIZE_TYPE length, SYSTEM_TIME_TYPE time_out);
RETURN_CODE_TYPE sys_receive_queuing_message(QUEUING_PORT_ID_TYPE queuing_port_id,
                                             SYSTEM_TIME_TYPE time_out,
                                             MESSAGE_ADDR_TYPE message_addr,
                                             MESSAGE_SIZE_TYPE *length);
RETURN_CODE_TYPE sys_get_queuing_port_id(QUEUING_PORT_NAME_TYPE queuing_port_name,
                                         QUEUING_PORT_ID_TYPE *queuing_port_id);
RETURN_CODE_TYPE sys_get_queuing_port_status(QUEUING_PORT_ID_TYPE queuing_port_id,
                                             QUEUING_PORT_STATUS_TYPE *queuing_port_status);
RETURN_CODE_TYPE sys_clear_queuing_port(QUEUING_PORT_ID_TYPE queuing_port_id);
RETURN_CODE_TYPE sys_create_buffer(BUFFER_NAME_TYPE buffer_name,
                                   MESSAGE_SIZE_TYPE max_message_size,
                                   MESSAGE_RANGE_TYPE max_nb_message,
                                   QUEUING_DISCIPLINE_TYPE queuing_discipline,
                                   BUFFER_ID_TYPE *buffer_id);
RETURN_CODE_TYPE sys_send_buffer(BUFFER_ID_TYPE buffer_id, MESSAGE_ADDR_TYPE message_addr,
                                 MESSAGE_SIZE_TYPE length, SYSTEM_TIME_TYPE time_out);
RETURN_CODE_TYPE sys_receive_buffer(BUFFER_ID_TYPE buffer_id, SYSTEM_TIME_TYPE time_out,
                                    MESSAGE_ADDR_TYPE message_addr, MESSAGE_SIZE_TYPE *length);
RETURN_CODE_TYPE sys_get_buffer_id(BUFFER_NAME_TYPE buffer_name, BUFFER_ID_TYPE *buffer_id);
RETURN_CODE_TYPE sys_get_buffer_status(BUFFER_ID_TYPE buffer_id,
                                       BUFFER_STATUS_TYPE *buffer_status);
RETURN_CODE_TYPE sys_create_blackboard(BLACKBOARD_NAME_TYPE blackboard_name,
                                       MESSAGE_SIZE_TYPE max_message_size,
                                       BLACKBOARD_ID_TYPE *blackboard_id);
RETURN_CODE_TYPE sys_display_blackboard(BLACKBOARD_ID_TYPE blackboard_id,
                                        MESSAGE_ADDR_TYPE message_addr,
                                        MESSAGE_SIZE_TYPE length);
RETURN_CODE_TYPE sys_read_blackboard(BLACKBOARD_ID_TYPE blackboard_id,
                                     SYSTEM_TIME_TYPE time_out, MESSAGE_ADDR_TYPE message_addr,
                                     MESSAGE_SIZE_TYPE *length);
RETURN_CODE_TYPE sys_clear_blackboard(BLACKBOARD_ID_TYPE blackboard_id);
RETURN_CODE_TYPE sys_get_blackboard_id(BLACKBOARD_NAME_TYPE blackboard_name,
                                       BLACKBOARD_ID_TYPE *blackboard_id);
RETURN_CODE_TYPE sys_get_blackboard_status(BLACKBOARD_ID_TYPE blackboard_id,
                                           BLACKBOARD_STATUS_TYPE *blackboard_status);
RETURN_CODE_TYPE sys_create_semaphore(SEMAPHORE_NAME_TYPE semaphore_name,
                                      SEMAPHORE_VALUE_TYPE current_value,
                                      SEMAPHORE_VALUE_TYPE maximum_value,
                                      QUEUING_DISCIPLINE_TYPE queuing_discipline,
                                      SEMAPHORE_ID_TYPE *semaphore_id);
RETURN_CODE_TYPE sys_wait_semaphore(SEMAPHORE_ID_TYPE semaphore_id, SYSTEM_TIME_TYPE time_out);
RETURN_CODE_TYPE sys_signal_semaphore(SEMAPHORE_ID_TYPE semaphore_id);
RETURN_CODE_TYPE sys_get_semaphore_id(SEMAPHORE_NAME_TYPE semaphore_name,
                                      SEMAPHORE_ID_TYPE *semaphore_id);
RETURN_CODE_TYPE sys_get_semaphore_status(SEMAPHORE_ID_TYPE semaphore_id,
                                          SEMAPHORE_STATUS_TYPE *semaphore_status);
RETURN_CODE_TYPE sys_create_event(EVENT_NAME_TYPE event_name, EVENT_ID_TYPE *event_id);
RETURN_CODE_TYPE sys_set_event(EVENT_ID_TYPE event_id);
RETURN_CODE_TYPE sys_reset_event(EVENT_ID_TYPE event_id);
RETURN_CODE_TYPE sys_wait_event(EVENT_ID_TYPE event_id, SYSTEM_TIME_TYPE time_out);
RETURN_CODE_TYPE sys_get_event_id(EVENT_NAME_TYPE event_name, EVENT_ID_TYPE *event_id);
RETURN_CODE_TYPE sys_get_event_status(EVENT_ID_TYPE event_id, EVENT_STATUS_TYPE *event_status);
RETURN_CODE_TYPE sys_create_mutex(MUTEX_NAME_TYPE mutex_name, PRIORITY_TYPE mutex_priority,
                                  QUEUING_DISCIPLINE_TYPE queuing_discipline,
                                  MUTEX_ID_TYPE *mutex_id);
RETURN_CODE_TYPE sys_acquire_mutex(MUTEX_ID_TYPE mutex_id, SYSTEM_TIME_TYPE time_out);
RETURN_CODE_TYPE sys_release_mutex(MUTEX_ID_TYPE mutex_id);
RETURN_CODE_TYPE sys_reset_mutex(MUTEX_ID_TYPE mutex_id, PROCESS_ID_TYPE process_id);
RETURN_CODE_TYPE sys_get_mutex_id(MUTEX_NAME_TYPE mutex_name, MUTEX_ID_TYPE *mutex_id);
RETURN_CODE_TYPE sys_get_mutex_status(MUTEX_ID_TYPE mutex_id, MUTEX_STATUS_TYPE *mutex_status);
RETURN_CODE_TYPE sys_get_process_mutex_state(PROCESS_ID_TYPE process_id,
                                             MUTEX_ID_TYPE *mutex_id);

#endif

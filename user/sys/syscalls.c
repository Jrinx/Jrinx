#include <stddef.h>
#include <sysno.h>
#include <types.h>

static inline unsigned long syscall(unsigned long sysno, unsigned long arg0, unsigned long arg1,
                                    unsigned long arg2, unsigned long arg3, unsigned long arg4,
                                    unsigned long arg5, unsigned long arg6) {
  register long a0 asm("a0") = arg0;
  register long a1 asm("a1") = arg1;
  register long a2 asm("a2") = arg2;
  register long a3 asm("a3") = arg3;
  register long a4 asm("a4") = arg4;
  register long a5 asm("a5") = arg5;
  register long a6 asm("a6") = arg6;
  register long a7 asm("a7") = sysno;
  asm volatile("ecall"
               : "+r"(a0), "+r"(a1)
               : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7)
               : "memory");
  return a0;
}

void sys_cons_write_char(int ch) {
  syscall(SYS_CONS_WRITE_CHAR, ch, 0, 0, 0, 0, 0, 0);
}

int sys_cons_read_char(void) {
  return syscall(SYS_CONS_READ_CHAR, 0, 0, 0, 0, 0, 0, 0);
}

void sys_cons_write_buf(const char *buf, size_t len) {
  syscall(SYS_CONS_WRITE_BUF, (unsigned long)buf, len, 0, 0, 0, 0, 0);
}

void sys_halt(void) {
  syscall(SYS_HALT, 0, 0, 0, 0, 0, 0, 0);
  __builtin_unreachable();
}

RETURN_CODE_TYPE sys_get_partition_status(PARTITION_STATUS_TYPE *partition_status) {
  return syscall(SYS_GET_PARTITION_STATUS, (unsigned long)partition_status, 0, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_set_partition_mode(OPERATING_MODE_TYPE operating_mode) {
  return syscall(SYS_SET_PARTITION_MODE, operating_mode, 0, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_get_process_id(PROCESS_NAME_TYPE process_name,
                                    PROCESS_ID_TYPE *process_id) {
  return syscall(SYS_GET_PROCESS_ID, (unsigned long)process_name, (unsigned long)process_id, 0,
                 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_get_process_status(PROCESS_ID_TYPE process_id,
                                        PROCESS_STATUS_TYPE *process_status) {
  return syscall(SYS_GET_PROCESS_STATUS, process_id, (unsigned long)process_status, 0, 0, 0, 0,
                 0);
}

RETURN_CODE_TYPE sys_create_process(PROCESS_ATTRIBUTE_TYPE *attributes,
                                    PROCESS_ID_TYPE *process_id) {
  return syscall(SYS_CREATE_PROCESS, (unsigned long)attributes, (unsigned long)process_id, 0, 0,
                 0, 0, 0);
}

RETURN_CODE_TYPE sys_set_priority(PROCESS_ID_TYPE process_id, PRIORITY_TYPE priority) {
  return syscall(SYS_SET_PRIORITY, process_id, priority, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_suspend_self(SYSTEM_TIME_TYPE time_out) {
  return syscall(SYS_SUSPEND_SELF, time_out, 0, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_suspend(PROCESS_ID_TYPE process_id) {
  return syscall(SYS_SUSPEND, process_id, 0, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_resume(PROCESS_ID_TYPE process_id) {
  return syscall(SYS_RESUME, process_id, 0, 0, 0, 0, 0, 0);
}

void sys_stop_self(void) {
  syscall(SYS_STOP_SELF, 0, 0, 0, 0, 0, 0, 0);
  __builtin_unreachable();
}

RETURN_CODE_TYPE sys_stop(PROCESS_ID_TYPE process_id) {
  return syscall(SYS_STOP, process_id, 0, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_start(PROCESS_ID_TYPE process_id) {
  return syscall(SYS_START, process_id, 0, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_delayed_start(PROCESS_ID_TYPE process_id, SYSTEM_TIME_TYPE delay_time) {
  return syscall(SYS_DELAYED_START, process_id, delay_time, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_lock_preemption(LOCK_LEVEL_TYPE *lock_level) {
  return syscall(SYS_LOCK_PREEMPTION, (unsigned long)lock_level, 0, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_unlock_preemption(LOCK_LEVEL_TYPE *lock_level) {
  return syscall(SYS_UNLOCK_PREEMPTION, (unsigned long)lock_level, 0, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_get_my_id(PROCESS_ID_TYPE *process_id) {
  return syscall(SYS_GET_MY_ID, (unsigned long)process_id, 0, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE
sys_initialize_process_core_affinity(PROCESS_ID_TYPE process_id,
                                     PROCESSOR_CORE_ID_TYPE processor_core_id) {
  return syscall(SYS_INITIALIZE_PROCESS_CORE_AFFINITY, process_id, processor_core_id, 0, 0, 0,
                 0, 0);
}

RETURN_CODE_TYPE sys_get_my_processor_core_id(PROCESSOR_CORE_ID_TYPE *processor_core_id) {
  return syscall(SYS_GET_MY_PROCESSOR_CORE_ID, (unsigned long)processor_core_id, 0, 0, 0, 0, 0,
                 0);
}

RETURN_CODE_TYPE sys_get_my_index(PROCESS_INDEX_TYPE *process_index) {
  return syscall(SYS_GET_MY_INDEX, (unsigned long)process_index, 0, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_timed_wait(SYSTEM_TIME_TYPE delay_time) {
  return syscall(SYS_TIMED_WAIT, delay_time, 0, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_periodic_wait(void) {
  return syscall(SYS_PERIODIC_WAIT, 0, 0, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_get_time(SYSTEM_TIME_TYPE *system_time) {
  return syscall(SYS_GET_TIME, (unsigned long)system_time, 0, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_replenish(SYSTEM_TIME_TYPE budget_time) {
  return syscall(SYS_REPLENISH, budget_time, 0, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_create_sampling_port(SAMPLING_PORT_NAME_TYPE sampling_port_name,
                                          MESSAGE_SIZE_TYPE max_message_size,
                                          PORT_DIRECTION_TYPE port_direction,
                                          SYSTEM_TIME_TYPE refresh_period,
                                          SAMPLING_PORT_ID_TYPE *sampling_port_id) {
  return syscall(SYS_CREATE_SAMPLING_PORT, (unsigned long)sampling_port_name, max_message_size,
                 port_direction, refresh_period, (unsigned long)sampling_port_id, 0, 0);
}

RETURN_CODE_TYPE sys_write_sampling_message(SAMPLING_PORT_ID_TYPE sampling_port_id,
                                            MESSAGE_ADDR_TYPE message_addr,
                                            MESSAGE_SIZE_TYPE length) {
  return syscall(SYS_WRITE_SAMPLING_MESSAGE, sampling_port_id, (unsigned long)message_addr,
                 length, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_read_sampling_message(SAMPLING_PORT_ID_TYPE sampling_port_id,
                                           MESSAGE_ADDR_TYPE message_addr,
                                           MESSAGE_SIZE_TYPE *length, VALIDITY_TYPE *validity) {
  return syscall(SYS_READ_SAMPLING_MESSAGE, sampling_port_id, (unsigned long)message_addr, 0,
                 (unsigned long)length, (unsigned long)validity, 0, 0);
}

RETURN_CODE_TYPE sys_get_sampling_port_id(SAMPLING_PORT_NAME_TYPE sampling_port_name,
                                          SAMPLING_PORT_ID_TYPE *sampling_port_id) {
  return syscall(SYS_GET_SAMPLING_PORT_ID, (unsigned long)sampling_port_name,
                 (unsigned long)sampling_port_id, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_get_sampling_port_status(SAMPLING_PORT_ID_TYPE sampling_port_id,
                                              SAMPLING_PORT_STATUS_TYPE *sampling_port_status) {
  return syscall(SYS_GET_SAMPLING_PORT_STATUS, sampling_port_id,
                 (unsigned long)sampling_port_status, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_create_queuing_port(QUEUING_PORT_NAME_TYPE queuing_port_name,
                                         MESSAGE_SIZE_TYPE max_message_size,
                                         MESSAGE_RANGE_TYPE max_nb_message,
                                         PORT_DIRECTION_TYPE port_direction,
                                         QUEUING_DISCIPLINE_TYPE queuing_discipline,
                                         QUEUING_PORT_ID_TYPE *queuing_port_id) {
  return syscall(SYS_CREATE_QUEUING_PORT, (unsigned long)queuing_port_name, max_message_size,
                 max_nb_message, port_direction, queuing_discipline,
                 (unsigned long)queuing_port_id, 0);
}

RETURN_CODE_TYPE sys_send_queuing_message(QUEUING_PORT_ID_TYPE queuing_port_id,
                                          MESSAGE_ADDR_TYPE message_addr,
                                          MESSAGE_SIZE_TYPE length, SYSTEM_TIME_TYPE time_out) {
  return syscall(SYS_SEND_QUEUING_MESSAGE, queuing_port_id, (unsigned long)message_addr, length,
                 time_out, 0, 0, 0);
}

RETURN_CODE_TYPE sys_receive_queuing_message(QUEUING_PORT_ID_TYPE queuing_port_id,
                                             SYSTEM_TIME_TYPE time_out,
                                             MESSAGE_ADDR_TYPE message_addr,
                                             MESSAGE_SIZE_TYPE *length) {
  return syscall(SYS_RECEIVE_QUEUING_MESSAGE, queuing_port_id, time_out,
                 (unsigned long)message_addr, (unsigned long)length, 0, 0, 0);
}

RETURN_CODE_TYPE sys_get_queuing_port_id(QUEUING_PORT_NAME_TYPE queuing_port_name,
                                         QUEUING_PORT_ID_TYPE *queuing_port_id) {
  return syscall(SYS_GET_QUEUING_PORT_ID, (unsigned long)queuing_port_name,
                 (unsigned long)queuing_port_id, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_get_queuing_port_status(QUEUING_PORT_ID_TYPE queuing_port_id,
                                             QUEUING_PORT_STATUS_TYPE *queuing_port_status) {
  return syscall(SYS_GET_QUEUING_PORT_STATUS, queuing_port_id,
                 (unsigned long)queuing_port_status, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_clear_queuing_port(QUEUING_PORT_ID_TYPE queuing_port_id) {
  return syscall(SYS_CLEAR_QUEUING_PORT, queuing_port_id, 0, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_create_buffer(BUFFER_NAME_TYPE buffer_name,
                                   MESSAGE_SIZE_TYPE max_message_size,
                                   MESSAGE_RANGE_TYPE max_nb_message,
                                   QUEUING_DISCIPLINE_TYPE queuing_discipline,
                                   BUFFER_ID_TYPE *buffer_id) {
  return syscall(SYS_CREATE_BUFFER, (unsigned long)buffer_name, max_message_size,
                 max_nb_message, queuing_discipline, (unsigned long)buffer_id, 0, 0);
}

RETURN_CODE_TYPE sys_send_buffer(BUFFER_ID_TYPE buffer_id, MESSAGE_ADDR_TYPE message_addr,
                                 MESSAGE_SIZE_TYPE length, SYSTEM_TIME_TYPE time_out) {
  return syscall(SYS_SEND_BUFFER, buffer_id, (unsigned long)message_addr, length, time_out, 0,
                 0, 0);
}

RETURN_CODE_TYPE sys_receive_buffer(BUFFER_ID_TYPE buffer_id, SYSTEM_TIME_TYPE time_out,
                                    MESSAGE_ADDR_TYPE message_addr, MESSAGE_SIZE_TYPE *length) {
  return syscall(SYS_RECEIVE_BUFFER, buffer_id, time_out, (unsigned long)message_addr,
                 (unsigned long)length, 0, 0, 0);
}

RETURN_CODE_TYPE sys_get_buffer_id(BUFFER_NAME_TYPE buffer_name, BUFFER_ID_TYPE *buffer_id) {
  return syscall(SYS_GET_BUFFER_ID, (unsigned long)buffer_name, (unsigned long)buffer_id, 0, 0,
                 0, 0, 0);
}

RETURN_CODE_TYPE sys_get_buffer_status(BUFFER_ID_TYPE buffer_id,
                                       BUFFER_STATUS_TYPE *buffer_status) {
  return syscall(SYS_GET_BUFFER_STATUS, buffer_id, (unsigned long)buffer_status, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_create_blackboard(BLACKBOARD_NAME_TYPE blackboard_name,
                                       MESSAGE_SIZE_TYPE max_message_size,
                                       BLACKBOARD_ID_TYPE *blackboard_id) {
  return syscall(SYS_CREATE_BLACKBOARD, (unsigned long)blackboard_name, max_message_size, 0, 0,
                 0, 0, 0);
}

RETURN_CODE_TYPE sys_display_blackboard(BLACKBOARD_ID_TYPE blackboard_id,
                                        MESSAGE_ADDR_TYPE message_addr,
                                        MESSAGE_SIZE_TYPE length) {
  return syscall(SYS_DISPLAY_BLACKBOARD, blackboard_id, (unsigned long)message_addr, length, 0,
                 0, 0, 0);
}

RETURN_CODE_TYPE sys_read_blackboard(BLACKBOARD_ID_TYPE blackboard_id,
                                     SYSTEM_TIME_TYPE time_out, MESSAGE_ADDR_TYPE message_addr,
                                     MESSAGE_SIZE_TYPE *length) {
  return syscall(SYS_READ_BLACKBOARD, blackboard_id, time_out, (unsigned long)message_addr,
                 (unsigned long)length, 0, 0, 0);
}

RETURN_CODE_TYPE sys_clear_blackboard(BLACKBOARD_ID_TYPE blackboard_id) {
  return syscall(SYS_CLEAR_BLACKBOARD, blackboard_id, 0, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_get_blackboard_id(BLACKBOARD_NAME_TYPE blackboard_name,
                                       BLACKBOARD_ID_TYPE *blackboard_id) {
  return syscall(SYS_GET_BLACKBOARD_ID, (unsigned long)blackboard_name,
                 (unsigned long)blackboard_id, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_get_blackboard_status(BLACKBOARD_ID_TYPE blackboard_id,
                                           BLACKBOARD_STATUS_TYPE *blackboard_status) {
  return syscall(SYS_GET_BLACKBOARD_STATUS, blackboard_id, (unsigned long)blackboard_status, 0,
                 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_create_semaphore(SEMAPHORE_NAME_TYPE semaphore_name,
                                      SEMAPHORE_VALUE_TYPE current_value,
                                      SEMAPHORE_VALUE_TYPE maximum_value,
                                      QUEUING_DISCIPLINE_TYPE queuing_discipline,
                                      SEMAPHORE_ID_TYPE *semaphore_id) {
  return syscall(SYS_CREATE_SEMAPHORE, (unsigned long)semaphore_name, current_value,
                 maximum_value, queuing_discipline, (unsigned long)semaphore_id, 0, 0);
}

RETURN_CODE_TYPE sys_wait_semaphore(SEMAPHORE_ID_TYPE semaphore_id, SYSTEM_TIME_TYPE time_out) {
  return syscall(SYS_WAIT_SEMAPHORE, semaphore_id, time_out, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_signal_semaphore(SEMAPHORE_ID_TYPE semaphore_id) {
  return syscall(SYS_SIGNAL_SEMAPHORE, semaphore_id, 0, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_get_semaphore_id(SEMAPHORE_NAME_TYPE semaphore_name,
                                      SEMAPHORE_ID_TYPE *semaphore_id) {
  return syscall(SYS_GET_SEMAPHORE_ID, (unsigned long)semaphore_name,
                 (unsigned long)semaphore_id, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_get_semaphore_status(SEMAPHORE_ID_TYPE semaphore_id,
                                          SEMAPHORE_STATUS_TYPE *semaphore_status) {
  return syscall(SYS_GET_SEMAPHORE_STATUS, semaphore_id, (unsigned long)semaphore_status, 0, 0,
                 0, 0, 0);
}

RETURN_CODE_TYPE sys_create_event(EVENT_NAME_TYPE event_name, EVENT_ID_TYPE *event_id) {
  return syscall(SYS_CREATE_EVENT, (unsigned long)event_name, (unsigned long)event_id, 0, 0, 0,
                 0, 0);
}

RETURN_CODE_TYPE sys_set_event(EVENT_ID_TYPE event_id) {
  return syscall(SYS_SET_EVENT, event_id, 0, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_reset_event(EVENT_ID_TYPE event_id) {
  return syscall(SYS_RESET_EVENT, event_id, 0, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_wait_event(EVENT_ID_TYPE event_id, SYSTEM_TIME_TYPE time_out) {
  return syscall(SYS_WAIT_EVENT, event_id, time_out, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_get_event_id(EVENT_NAME_TYPE event_name, EVENT_ID_TYPE *event_id) {
  return syscall(SYS_GET_EVENT_ID, (unsigned long)event_name, (unsigned long)event_id, 0, 0, 0,
                 0, 0);
}

RETURN_CODE_TYPE sys_get_event_status(EVENT_ID_TYPE event_id, EVENT_STATUS_TYPE *event_status) {
  return syscall(SYS_GET_EVENT_STATUS, event_id, (unsigned long)event_status, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_create_mutex(MUTEX_NAME_TYPE mutex_name, PRIORITY_TYPE mutex_priority,
                                  QUEUING_DISCIPLINE_TYPE queuing_discipline,
                                  MUTEX_ID_TYPE *mutex_id) {
  return syscall(SYS_CREATE_MUTEX, (unsigned long)mutex_name, mutex_priority,
                 queuing_discipline, (unsigned long)mutex_id, 0, 0, 0);
}

RETURN_CODE_TYPE sys_acquire_mutex(MUTEX_ID_TYPE mutex_id, SYSTEM_TIME_TYPE time_out) {
  return syscall(SYS_ACQUIRE_MUTEX, mutex_id, time_out, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_release_mutex(MUTEX_ID_TYPE mutex_id) {
  return syscall(SYS_RELEASE_MUTEX, mutex_id, 0, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_reset_mutex(MUTEX_ID_TYPE mutex_id, PROCESS_ID_TYPE process_id) {
  return syscall(SYS_RESET_MUTEX, mutex_id, process_id, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_get_mutex_id(MUTEX_NAME_TYPE mutex_name, MUTEX_ID_TYPE *mutex_id) {
  return syscall(SYS_GET_MUTEX_ID, (unsigned long)mutex_name, (unsigned long)mutex_id, 0, 0, 0,
                 0, 0);
}

RETURN_CODE_TYPE sys_get_mutex_status(MUTEX_ID_TYPE mutex_id, MUTEX_STATUS_TYPE *mutex_status) {
  return syscall(SYS_GET_MUTEX_STATUS, mutex_id, (unsigned long)mutex_status, 0, 0, 0, 0, 0);
}

RETURN_CODE_TYPE sys_get_process_mutex_state(PROCESS_ID_TYPE process_id,
                                             MUTEX_ID_TYPE *mutex_id) {
  return syscall(SYS_GET_PROCESS_MUTEX_STATE, process_id, (unsigned long)mutex_id, 0, 0, 0, 0,
                 0);
}

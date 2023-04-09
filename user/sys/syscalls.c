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

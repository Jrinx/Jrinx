#ifndef _USER_SYS_SERVICES_H_
#define _USER_SYS_SERVICES_H_

#include <types.h>
#include <user/sys/syscalls.h>

static inline void GET_PARTITION_STATUS(/* in */ PARTITION_STATUS_TYPE *PARTITION_STATUS,
                                        /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_get_partition_status(PARTITION_STATUS);
}

static inline void SET_PARTITION_MODE(/* in */ OPERATING_MODE_TYPE OPERATING_MODE,
                                      /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_set_partition_mode(OPERATING_MODE);
}

static inline void GET_PROCESS_ID(/* in */ PROCESS_NAME_TYPE PROCESS_NAME,
                                  /* out */ PROCESS_ID_TYPE *PROCESS_ID,
                                  /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_get_process_id(PROCESS_NAME, PROCESS_ID);
}

static inline void GET_PROCESS_STATUS(/* in */ PROCESS_ID_TYPE PROCESS_ID,
                                      /* out */ PROCESS_STATUS_TYPE *PROCESS_STATUS,
                                      /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_get_process_status(PROCESS_ID, PROCESS_STATUS);
}

static inline void CREATE_PROCESS(/* in */ PROCESS_ATTRIBUTE_TYPE *ATTRIBUTES,
                                  /* out */ PROCESS_ID_TYPE *PROCESS_ID,
                                  /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_create_process(ATTRIBUTES, PROCESS_ID);
}

static inline void SET_PRIORITY(/* in */ PROCESS_ID_TYPE PROCESS_ID,
                                /* in */ PRIORITY_TYPE PRIORITY,
                                /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_set_priority(PROCESS_ID, PRIORITY);
}

static inline void SUSPEND_SELF(/* in */ SYSTEM_TIME_TYPE TIME_OUT,
                                /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_suspend_self(TIME_OUT);
}

static inline void SUSPEND(/* in */ PROCESS_ID_TYPE PROCESS_ID,
                           /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_suspend(PROCESS_ID);
}

static inline void RESUME(/* in */ PROCESS_ID_TYPE PROCESS_ID,
                          /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_resume(PROCESS_ID);
}

static inline void STOP_SELF(void) {
  sys_stop_self();
}

static inline void STOP(/* in */ PROCESS_ID_TYPE PROCESS_ID,
                        /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_stop(PROCESS_ID);
}

static inline void START(/* in */ PROCESS_ID_TYPE PROCESS_ID,
                         /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_start(PROCESS_ID);
}

static inline void DELAYED_START(/* in */ PROCESS_ID_TYPE PROCESS_ID,
                                 /* in */ SYSTEM_TIME_TYPE DELAY_TIME,
                                 /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_delayed_start(PROCESS_ID, DELAY_TIME);
}

static inline void LOCK_PREEMPTION(/* out */ LOCK_LEVEL_TYPE *LOCK_LEVEL,
                                   /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_lock_preemption(LOCK_LEVEL);
}

static inline void UNLOCK_PREEMPTION(/* out */ LOCK_LEVEL_TYPE *LOCK_LEVEL,
                                     /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_unlock_preemption(LOCK_LEVEL);
}

static inline void GET_MY_ID(/* out */ PROCESS_ID_TYPE *PROCESS_ID,
                             /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_get_my_id(PROCESS_ID);
}

static inline void
INITIALIZE_PROCESS_CORE_AFFINITY(/* in */ PROCESS_ID_TYPE PROCESS_ID,
                                 /* in */ PROCESSOR_CORE_ID_TYPE PROCESSOR_CORE_ID,
                                 /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_initialize_process_core_affinity(PROCESS_ID, PROCESSOR_CORE_ID);
}

static inline void GET_MY_PROCESSOR_CORE_ID(/* out */ PROCESSOR_CORE_ID_TYPE *PROCESSOR_CORE_ID,
                                            /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_get_my_processor_core_id(PROCESSOR_CORE_ID);
}

static inline void GET_MY_INDEX(/* out */ PROCESS_INDEX_TYPE *PROCESS_INDEX,
                                /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_get_my_index(PROCESS_INDEX);
}

static inline void TIMED_WAIT(/* in */ SYSTEM_TIME_TYPE DELAY_TIME,
                              /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_timed_wait(DELAY_TIME);
}

static inline void PERIODIC_WAIT(/* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_periodic_wait();
}

static inline void GET_TIME(/* out */ SYSTEM_TIME_TYPE *SYSTEM_TIME,
                            /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_get_time(SYSTEM_TIME);
}

static inline void REPLENISH(/* in */ SYSTEM_TIME_TYPE BUDGET_TIME,
                             /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_replenish(BUDGET_TIME);
}

#endif

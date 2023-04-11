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

static inline void CREATE_SAMPLING_PORT(/* in */ SAMPLING_PORT_NAME_TYPE SAMPLING_PORT_NAME,
                                        /* in */ MESSAGE_SIZE_TYPE MAX_MESSAGE_SIZE,
                                        /* in */ PORT_DIRECTION_TYPE PORT_DIRECTION,
                                        /* in */ SYSTEM_TIME_TYPE REFRESH_PERIOD,
                                        /* out */ SAMPLING_PORT_ID_TYPE *SAMPLING_PORT_ID,
                                        /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_create_sampling_port(SAMPLING_PORT_NAME, MAX_MESSAGE_SIZE, PORT_DIRECTION,
                                          REFRESH_PERIOD, SAMPLING_PORT_ID);
}

static inline void WRITE_SAMPLING_MESSAGE(/* in */ SAMPLING_PORT_ID_TYPE SAMPLING_PORT_ID,
                                          /* in */ MESSAGE_ADDR_TYPE MESSAGE_ADDR,
                                          /* in */ MESSAGE_SIZE_TYPE LENGTH,
                                          /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_write_sampling_message(SAMPLING_PORT_ID, MESSAGE_ADDR, LENGTH);
}

static inline void READ_SAMPLING_MESSAGE(/* in */ SAMPLING_PORT_ID_TYPE SAMPLING_PORT_ID,
                                         /* in */ MESSAGE_ADDR_TYPE MESSAGE_ADDR,
                                         /* out */ MESSAGE_SIZE_TYPE *LENGTH,
                                         /* out */ VALIDITY_TYPE *VALIDITY,
                                         /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_read_sampling_message(SAMPLING_PORT_ID, MESSAGE_ADDR, LENGTH, VALIDITY);
}

static inline void GET_SAMPLING_PORT_ID(/* in */ SAMPLING_PORT_NAME_TYPE SAMPLING_PORT_NAME,
                                        /* out */ SAMPLING_PORT_ID_TYPE *SAMPLING_PORT_ID,
                                        /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_get_sampling_port_id(SAMPLING_PORT_NAME, SAMPLING_PORT_ID);
}

static inline void
GET_SAMPLING_PORT_STATUS(/* in */ SAMPLING_PORT_ID_TYPE SAMPLING_PORT_ID,
                         /* out */ SAMPLING_PORT_STATUS_TYPE *SAMPLING_PORT_STATUS,
                         /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_get_sampling_port_status(SAMPLING_PORT_ID, SAMPLING_PORT_STATUS);
}

static inline void CREATE_QUEUING_PORT(/* in */ QUEUING_PORT_NAME_TYPE QUEUING_PORT_NAME,
                                       /* in */ MESSAGE_SIZE_TYPE MAX_MESSAGE_SIZE,
                                       /* in */ MESSAGE_RANGE_TYPE MAX_NB_MESSAGE,
                                       /* in */ PORT_DIRECTION_TYPE PORT_DIRECTION,
                                       /* in */ QUEUING_DISCIPLINE_TYPE QUEUING_DISCIPLINE,
                                       /* out */ QUEUING_PORT_ID_TYPE *QUEUING_PORT_ID,
                                       /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_create_queuing_port(QUEUING_PORT_NAME, MAX_MESSAGE_SIZE, MAX_NB_MESSAGE,
                                         PORT_DIRECTION, QUEUING_DISCIPLINE, QUEUING_PORT_ID);
}

static inline void SEND_QUEUING_MESSAGE(/* in */ QUEUING_PORT_ID_TYPE QUEUING_PORT_ID,
                                        /* in */ MESSAGE_ADDR_TYPE MESSAGE_ADDR,
                                        /* in */ MESSAGE_SIZE_TYPE LENGTH,
                                        /* in */ SYSTEM_TIME_TYPE TIME_OUT,
                                        /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_send_queuing_message(QUEUING_PORT_ID, MESSAGE_ADDR, LENGTH, TIME_OUT);
}

static inline void RECEIVE_QUEUING_MESSAGE(/* in */ QUEUING_PORT_ID_TYPE QUEUING_PORT_ID,
                                           /* in */ SYSTEM_TIME_TYPE TIME_OUT,
                                           /* in */ MESSAGE_ADDR_TYPE MESSAGE_ADDR,
                                           /* out */ MESSAGE_SIZE_TYPE *LENGTH,
                                           /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_receive_queuing_message(QUEUING_PORT_ID, TIME_OUT, MESSAGE_ADDR, LENGTH);
}

static inline void GET_QUEUING_PORT_ID(/* in */ QUEUING_PORT_NAME_TYPE QUEUING_PORT_NAME,
                                       /* out */ QUEUING_PORT_ID_TYPE *QUEUING_PORT_ID,
                                       /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_get_queuing_port_id(QUEUING_PORT_NAME, QUEUING_PORT_ID);
}

static inline void
GET_QUEUING_PORT_STATUS(/* in */ QUEUING_PORT_ID_TYPE QUEUING_PORT_ID,
                        /* out */ QUEUING_PORT_STATUS_TYPE *QUEUING_PORT_STATUS,
                        /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_get_queuing_port_status(QUEUING_PORT_ID, QUEUING_PORT_STATUS);
}

static inline void CLEAR_QUEUING_PORT(/* in */ QUEUING_PORT_ID_TYPE QUEUING_PORT_ID,
                                      /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_clear_queuing_port(QUEUING_PORT_ID);
}

static inline void CREATE_BUFFER(/* in */ BUFFER_NAME_TYPE BUFFER_NAME,
                                 /* in */ MESSAGE_SIZE_TYPE MAX_MESSAGE_SIZE,
                                 /* in */ MESSAGE_RANGE_TYPE MAX_NB_MESSAGE,
                                 /* in */ QUEUING_DISCIPLINE_TYPE QUEUING_DISCIPLINE,
                                 /* out */ BUFFER_ID_TYPE *BUFFER_ID,
                                 /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_create_buffer(BUFFER_NAME, MAX_MESSAGE_SIZE, MAX_NB_MESSAGE,
                                   QUEUING_DISCIPLINE, BUFFER_ID);
}

static inline void SEND_BUFFER(/* in */ BUFFER_ID_TYPE BUFFER_ID,
                               /* in */ MESSAGE_ADDR_TYPE MESSAGE_ADDR,
                               /* in */ MESSAGE_SIZE_TYPE LENGTH,
                               /* in */ SYSTEM_TIME_TYPE TIME_OUT,
                               /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_send_buffer(BUFFER_ID, MESSAGE_ADDR, LENGTH, TIME_OUT);
}

static inline void RECEIVE_BUFFER(/* in */ BUFFER_ID_TYPE BUFFER_ID,
                                  /* in */ SYSTEM_TIME_TYPE TIME_OUT,
                                  /* in */ MESSAGE_ADDR_TYPE MESSAGE_ADDR,
                                  /* out */ MESSAGE_SIZE_TYPE *LENGTH,
                                  /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_receive_buffer(BUFFER_ID, TIME_OUT, MESSAGE_ADDR, LENGTH);
}

static inline void GET_BUFFER_ID(/* in */ BUFFER_NAME_TYPE BUFFER_NAME,
                                 /* out */ BUFFER_ID_TYPE *BUFFER_ID,
                                 /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_get_buffer_id(BUFFER_NAME, BUFFER_ID);
}

static inline void GET_BUFFER_STATUS(/* in */ BUFFER_ID_TYPE BUFFER_ID,
                                     /* out */ BUFFER_STATUS_TYPE *BUFFER_STATUS,
                                     /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_get_buffer_status(BUFFER_ID, BUFFER_STATUS);
}

static inline void CREATE_BLACKBOARD(/* in */ BLACKBOARD_NAME_TYPE BLACKBOARD_NAME,
                                     /* in */ MESSAGE_SIZE_TYPE MAX_MESSAGE_SIZE,
                                     /* out */ BLACKBOARD_ID_TYPE *BLACKBOARD_ID,
                                     /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_create_blackboard(BLACKBOARD_NAME, MAX_MESSAGE_SIZE, BLACKBOARD_ID);
}

static inline void DISPLAY_BLACKBOARD(/* in */ BLACKBOARD_ID_TYPE BLACKBOARD_ID,
                                      /* in */ MESSAGE_ADDR_TYPE MESSAGE_ADDR,
                                      /* in */ MESSAGE_SIZE_TYPE LENGTH,
                                      /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_display_blackboard(BLACKBOARD_ID, MESSAGE_ADDR, LENGTH);
}

static inline void READ_BLACKBOARD(/* in */ BLACKBOARD_ID_TYPE BLACKBOARD_ID,
                                   /* in */ SYSTEM_TIME_TYPE TIME_OUT,
                                   /* in */ MESSAGE_ADDR_TYPE MESSAGE_ADDR,
                                   /* out */ MESSAGE_SIZE_TYPE *LENGTH,
                                   /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_read_blackboard(BLACKBOARD_ID, TIME_OUT, MESSAGE_ADDR, LENGTH);
}

static inline void CLEAR_BLACKBOARD(/* in */ BLACKBOARD_ID_TYPE BLACKBOARD_ID,
                                    /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_clear_blackboard(BLACKBOARD_ID);
}

static inline void GET_BLACKBOARD_ID(/* in */ BLACKBOARD_NAME_TYPE BLACKBOARD_NAME,
                                     /* out */ BLACKBOARD_ID_TYPE *BLACKBOARD_ID,
                                     /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_get_blackboard_id(BLACKBOARD_NAME, BLACKBOARD_ID);
}

static inline void GET_BLACKBOARD_STATUS(/* in */ BLACKBOARD_ID_TYPE BLACKBOARD_ID,
                                         /* out */ BLACKBOARD_STATUS_TYPE *BLACKBOARD_STATUS,
                                         /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_get_blackboard_status(BLACKBOARD_ID, BLACKBOARD_STATUS);
}

static inline void CREATE_SEMAPHORE(/* in */ SEMAPHORE_NAME_TYPE SEMAPHORE_NAME,
                                    /* in */ SEMAPHORE_VALUE_TYPE CURRENT_VALUE,
                                    /* in */ SEMAPHORE_VALUE_TYPE MAXIMUM_VALUE,
                                    /* in */ QUEUING_DISCIPLINE_TYPE QUEUING_DISCIPLINE,
                                    /* out */ SEMAPHORE_ID_TYPE *SEMAPHORE_ID,
                                    /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_create_semaphore(SEMAPHORE_NAME, CURRENT_VALUE, MAXIMUM_VALUE,
                                      QUEUING_DISCIPLINE, SEMAPHORE_ID);
}

static inline void WAIT_SEMAPHORE(/* in */ SEMAPHORE_ID_TYPE SEMAPHORE_ID,
                                  /* in */ SYSTEM_TIME_TYPE TIME_OUT,
                                  /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_wait_semaphore(SEMAPHORE_ID, TIME_OUT);
}

static inline void SIGNAL_SEMAPHORE(/* in */ SEMAPHORE_ID_TYPE SEMAPHORE_ID,
                                    /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_signal_semaphore(SEMAPHORE_ID);
}

static inline void GET_SEMAPHORE_ID(/* in */ SEMAPHORE_NAME_TYPE SEMAPHORE_NAME,
                                    /* out */ SEMAPHORE_ID_TYPE *SEMAPHORE_ID,
                                    /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_get_semaphore_id(SEMAPHORE_NAME, SEMAPHORE_ID);
}

static inline void GET_SEMAPHORE_STATUS(/* in */ SEMAPHORE_ID_TYPE SEMAPHORE_ID,
                                        /* out */ SEMAPHORE_STATUS_TYPE *SEMAPHORE_STATUS,
                                        /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_get_semaphore_status(SEMAPHORE_ID, SEMAPHORE_STATUS);
}

static inline void CREATE_EVENT(/* in */ EVENT_NAME_TYPE EVENT_NAME,
                                /* out */ EVENT_ID_TYPE *EVENT_ID,
                                /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_create_event(EVENT_NAME, EVENT_ID);
}

static inline void SET_EVENT(/* in */ EVENT_ID_TYPE EVENT_ID,
                             /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_set_event(EVENT_ID);
}

static inline void RESET_EVENT(/* in */ EVENT_ID_TYPE EVENT_ID,
                               /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_reset_event(EVENT_ID);
}

static inline void WAIT_EVENT(/* in */ EVENT_ID_TYPE EVENT_ID,
                              /* in */ SYSTEM_TIME_TYPE TIME_OUT,
                              /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_wait_event(EVENT_ID, TIME_OUT);
}

static inline void GET_EVENT_ID(/* in */ EVENT_NAME_TYPE EVENT_NAME,
                                /* out */ EVENT_ID_TYPE *EVENT_ID,
                                /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_get_event_id(EVENT_NAME, EVENT_ID);
}

static inline void GET_EVENT_STATUS(/* in */ EVENT_ID_TYPE EVENT_ID,
                                    /* out */ EVENT_STATUS_TYPE *EVENT_STATUS,
                                    /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_get_event_status(EVENT_ID, EVENT_STATUS);
}

static inline void CREATE_MUTEX(/* in */ MUTEX_NAME_TYPE MUTEX_NAME,
                                /* in */ PRIORITY_TYPE MUTEX_PRIORITY,
                                /* in */ QUEUING_DISCIPLINE_TYPE QUEUING_DISCIPLINE,
                                /* out */ MUTEX_ID_TYPE *MUTEX_ID,
                                /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_create_mutex(MUTEX_NAME, MUTEX_PRIORITY, QUEUING_DISCIPLINE, MUTEX_ID);
}

static inline void ACQUIRE_MUTEX(/* in */ MUTEX_ID_TYPE MUTEX_ID,
                                 /* in */ SYSTEM_TIME_TYPE TIME_OUT,
                                 /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_acquire_mutex(MUTEX_ID, TIME_OUT);
}

static inline void RELEASE_MUTEX(/* in */ MUTEX_ID_TYPE MUTEX_ID,
                                 /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_release_mutex(MUTEX_ID);
}

static inline void RESET_MUTEX(/* in */ MUTEX_ID_TYPE MUTEX_ID,
                               /* in */ PROCESS_ID_TYPE PROCESS_ID,
                               /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_reset_mutex(MUTEX_ID, PROCESS_ID);
}

static inline void GET_MUTEX_ID(/* in */ MUTEX_NAME_TYPE MUTEX_NAME,
                                /* out */ MUTEX_ID_TYPE *MUTEX_ID,
                                /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_get_mutex_id(MUTEX_NAME, MUTEX_ID);
}

static inline void GET_MUTEX_STATUS(/* in */ MUTEX_ID_TYPE MUTEX_ID,
                                    /* out */ MUTEX_STATUS_TYPE *MUTEX_STATUS,
                                    /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_get_mutex_status(MUTEX_ID, MUTEX_STATUS);
}

static inline void GET_PROCESS_MUTEX_STATE(/* in */ PROCESS_ID_TYPE PROCESS_ID,
                                           /* out */ MUTEX_ID_TYPE *MUTEX_ID,
                                           /* out */ RETURN_CODE_TYPE *RETURN_CODE) {
  *RETURN_CODE = sys_get_process_mutex_state(PROCESS_ID, MUTEX_ID);
}

#endif

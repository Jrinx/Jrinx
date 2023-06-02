#ifndef _TYPES_H_
#define _TYPES_H_

#include <stddef.h>
#include <stdint.h>

#define SYS_TIME_MICROSECOND (1L)
#define SYS_TIME_MILLISECOND (1000L * SYS_TIME_MICROSECOND)
#define SYS_TIME_SECOND (1000L * SYS_TIME_MILLISECOND)

typedef enum {
  NO_ERROR = 0,
  NO_ACTION = -1,
  NOT_AVAILABLE = -2,
  INVALID_PARAM = -3,
  INVALID_CONFIG = -4,
  INVALID_MODE = -5,
  TIMED_OUT = -6,
  INVALID_SYSNO = -4096,
} RETURN_CODE_TYPE,
    ret_code_t;

typedef enum {
  IDLE,
  COLD_START,
  WARM_START,
  NORMAL,
} OPERATING_MODE_TYPE,
    op_mode_t;

typedef enum {
  NORMAL_START,
  PARTITION_RESTART,
  HM_MODULE_RESTART,
  HM_PARTITION_RESTART,
} START_CONDITION_TYPE,
    start_cond_t;

typedef enum {
  DORMANT,
  READY,
  RUNNING,
  WAITING,
  FAULTED,
} PROCESS_STATE_TYPE,
    proc_state_t;

typedef enum {
  SOFT,
  HARD,
} DEADLINE_TYPE,
    deadline_t;

typedef enum {
  SOURCE,
  DESTINATION,
} PORT_DIRECTION_TYPE,
    port_dir_t;

typedef enum {
  INVALID,
  VALID,
} VALIDITY_TYPE,
    validity_t;

typedef enum {
  FIFO,
  PRIORITY,
} QUEUING_DISCIPLINE_TYPE,
    que_disc_t;

typedef enum {
  EMPTY,
  OCCUPIED,
} EMPTY_INDICATOR_TYPE,
    empty_ind_t;

typedef enum {
  DOWN,
  UP,
} EVENT_STATE_TYPE,
    event_state_t;

typedef enum {
  AVAILABLE,
  OWNED,
} MUTEX_STATE_TYPE,
    mutex_state_t;

#define SYSTEM_TIME_INFINITE_VAL INTMAX_MAX

typedef intmax_t SYSTEM_TIME_TYPE, sys_time_t;
typedef uintmax_t PARTITION_ID_TYPE, part_id_t;
typedef uintmax_t LOCK_LEVEL_TYPE, lock_level_t;
typedef uintmax_t PROCESSOR_CORE_ID_TYPE, proc_core_id_t;
typedef size_t NUM_CORES_TYPE, num_cores_t;
typedef uintmax_t PROCESS_ID_TYPE, proc_id_t;
typedef char *PROCESS_NAME_TYPE, *proc_name_t;
typedef uintmax_t PRIORITY_TYPE, priority_t;
typedef size_t STACK_SIZE_TYPE, stack_size_t;
typedef unsigned long SYSTEM_ADDRESS_TYPE, sys_addr_t;
typedef uintmax_t PROCESS_INDEX_TYPE, proc_index_t;
typedef void *MESSAGE_ADDR_TYPE, *msg_addr_t;
typedef size_t MESSAGE_SIZE_TYPE, msg_size_t;
typedef uintmax_t SAMPLING_PORT_ID_TYPE, samp_port_id_t;
typedef char *SAMPLING_PORT_NAME_TYPE, *samp_port_name_t;
typedef uintmax_t QUEUING_PORT_ID_TYPE, que_port_id_t;
typedef char *QUEUING_PORT_NAME_TYPE, *que_port_name_t;
typedef uintmax_t MESSAGE_RANGE_TYPE, msg_range_t;
typedef uintmax_t WAITING_RANGE_TYPE, wait_range_t;
typedef char *BUFFER_NAME_TYPE, *buf_name_t;
typedef uintmax_t BUFFER_ID_TYPE, buf_id_t;
typedef char *BLACKBOARD_NAME_TYPE, *bb_name_t;
typedef uintmax_t BLACKBOARD_ID_TYPE, bb_id_t;
typedef char *SEMAPHORE_NAME_TYPE, *sem_name_t;
typedef uintmax_t SEMAPHORE_ID_TYPE, sem_id_t;
typedef uintmax_t SEMAPHORE_VALUE_TYPE, sem_value_t;
typedef char *EVENT_NAME_TYPE, *event_name_t;
typedef uintmax_t EVENT_ID_TYPE, event_id_t;
typedef char *MUTEX_NAME_TYPE, *mutex_name_t;
typedef uintmax_t MUTEX_ID_TYPE, mutex_id_t;
typedef uintmax_t LOCK_COUNT_TYPE, lock_count_t;

struct comm_msg {
  msg_size_t msg_size;
  uint8_t msg_data[];
};

typedef struct {
  SYSTEM_TIME_TYPE PERIOD;
  SYSTEM_TIME_TYPE DURATION;
  PARTITION_ID_TYPE IDENTIFIER;
  LOCK_LEVEL_TYPE LOCK_LEVEL;
  OPERATING_MODE_TYPE OPERATING_MODE;
  START_CONDITION_TYPE START_CONDITION;
  NUM_CORES_TYPE NUM_ASSIGNED_CORES;
} PARTITION_STATUS_TYPE;

typedef struct {
  SYSTEM_TIME_TYPE PERIOD;
  SYSTEM_TIME_TYPE TIME_CAPACITY;
  SYSTEM_ADDRESS_TYPE ENTRY_POINT;
  STACK_SIZE_TYPE STACK_SIZE;
  PRIORITY_TYPE BASE_PRIORITY;
  DEADLINE_TYPE DEADLINE;
  PROCESS_NAME_TYPE NAME;
} PROCESS_ATTRIBUTE_TYPE;

typedef struct {
  SYSTEM_TIME_TYPE DEADLINE_TIME;
  PRIORITY_TYPE CURRENT_PRIORITY;
  PROCESS_STATE_TYPE PROCESS_STATE;
  PROCESS_ATTRIBUTE_TYPE ATTRIBUTES;
} PROCESS_STATUS_TYPE;

typedef struct {
  SYSTEM_TIME_TYPE REFRESH_PERIOD;
  MESSAGE_SIZE_TYPE MAX_MESSAGE_SIZE;
  PORT_DIRECTION_TYPE PORT_DIRECTION;
  VALIDITY_TYPE LAST_MSG_VALIDITY;
} SAMPLING_PORT_STATUS_TYPE;

typedef struct {
  MESSAGE_RANGE_TYPE NB_MESSAGE;
  MESSAGE_RANGE_TYPE MAX_NB_MESSAGE;
  MESSAGE_SIZE_TYPE MAX_MESSAGE_SIZE;
  PORT_DIRECTION_TYPE PORT_DIRECTION;
  WAITING_RANGE_TYPE WAITING_PROCESSES;
} QUEUING_PORT_STATUS_TYPE;

typedef struct {
  MESSAGE_RANGE_TYPE NB_MESSAGE;
  MESSAGE_RANGE_TYPE MAX_NB_MESSAGE;
  MESSAGE_SIZE_TYPE MAX_MESSAGE_SIZE;
  WAITING_RANGE_TYPE WAITING_PROCESSES;
} BUFFER_STATUS_TYPE;

typedef struct {
  EMPTY_INDICATOR_TYPE EMPTY_INDICATOR;
  MESSAGE_SIZE_TYPE MAX_MESSAGE_SIZE;
  WAITING_RANGE_TYPE WAITING_PROCESSES;
} BLACKBOARD_STATUS_TYPE;

typedef struct {
  SEMAPHORE_VALUE_TYPE CURRENT_VALUE;
  SEMAPHORE_VALUE_TYPE MAXIMUM_VALUE;
  WAITING_RANGE_TYPE WAITING_PROCESSES;
} SEMAPHORE_STATUS_TYPE;

typedef struct {
  EVENT_STATE_TYPE EVENT_STATE;
  WAITING_RANGE_TYPE WAITING_PROCESSES;
} EVENT_STATUS_TYPE;

typedef struct {
  PROCESS_ID_TYPE MUTEX_OWNER;
  MUTEX_STATE_TYPE MUTEX_STATE;
  PRIORITY_TYPE MUTEX_PRIORITY;
  LOCK_COUNT_TYPE LOCK_COUNT;
  WAITING_RANGE_TYPE WAITING_PROCESSES;
} MUTEX_STATUS_TYPE;

#endif

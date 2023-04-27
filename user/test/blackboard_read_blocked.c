#include <layouts.h>
#include <user/lib/debug.h>
#include <user/lib/logger.h>
#include <user/sys/services.h>
#include <user/sys/syscalls.h>

void blackboard_read_check(void) {
  BLACKBOARD_ID_TYPE bb_id[2];
  check_e(GET_BLACKBOARD_ID("blackboard_0", &bb_id[0], &ret));
  check_e(GET_BLACKBOARD_ID("blackboard_1", &bb_id[1], &ret));
  PROCESS_ID_TYPE proc_id[2];
  check_e(GET_PROCESS_ID("blackboard_read_check_0", &proc_id[0], &ret));
  check_e(GET_PROCESS_ID("blackboard_read_check_1", &proc_id[1], &ret));
  PROCESS_ID_TYPE this;
  check_e(GET_MY_ID(&this, &ret));
  if (this == proc_id[0]) {
    for (uint32_t send = 0; send < 4; send++) {
      uint32_t recv;
      size_t recv_len;
      check_e(READ_BLACKBOARD(bb_id[1], SYSTEM_TIME_INFINITE_VAL, &recv, &recv_len, &ret));
      info("read %u (len=%lu) from blackboard %lu\n", recv, recv_len, bb_id[1]);
      info("clear blackboard %lu\n", bb_id[1]);
      check_e(CLEAR_BLACKBOARD(bb_id[1], &ret));
      info("display %u on blackboard %lu\n", send, bb_id[0]);
      check_e(DISPLAY_BLACKBOARD(bb_id[0], &send, sizeof(uint32_t), &ret));
    }
    STOP_SELF();
  } else {
    RETURN_CODE_TYPE ret;
    for (uint32_t send = 0;; send++) {
      info("display %u on blackboard %lu\n", send, bb_id[1]);
      check_e(DISPLAY_BLACKBOARD(bb_id[1], &send, sizeof(uint32_t), &ret));
      uint32_t recv;
      size_t recv_len;
      READ_BLACKBOARD(bb_id[0], 2 * SYS_TIME_SECOND, &recv, &recv_len, &ret);
      if (ret == NO_ERROR) {
        info("read %u (len=%lu) from blackboard %lu\n", recv, recv_len, bb_id[0]);
        info("clear blackboard %lu\n", bb_id[0]);
        check_e(CLEAR_BLACKBOARD(bb_id[0], &ret));
      } else if (ret == TIMED_OUT) {
        info("read from blackboard %lu timed out\n", bb_id[0]);
        break;
      }
    }
    BLACKBOARD_STATUS_TYPE status[2];
    check_e(GET_BLACKBOARD_STATUS(bb_id[0], &status[0], &ret));
    check_e(GET_BLACKBOARD_STATUS(bb_id[1], &status[1], &ret));
    info("blackboard %lu status: empty-ind=%s,max_msg_size:%lu,nb_waiting_proc=%lu\n", bb_id[0],
         status[0].EMPTY_INDICATOR == EMPTY ? "empty" : "occupied", status[0].MAX_MESSAGE_SIZE,
         status[0].WAITING_PROCESSES);
    info("blackboard %lu status: empty-ind=%s,max_msg_size:%lu,nb_waiting_proc=%lu\n", bb_id[1],
         status[1].EMPTY_INDICATOR == EMPTY ? "empty" : "occupied", status[1].MAX_MESSAGE_SIZE,
         status[1].WAITING_PROCESSES);
    halt("done\n");
  }
}

void main(void) {
  PROCESS_ATTRIBUTE_TYPE attr[2] = {
      {
          .NAME = "blackboard_read_check_0",
          .ENTRY_POINT = (SYSTEM_ADDRESS_TYPE)blackboard_read_check,
          .BASE_PRIORITY = 1,
          .STACK_SIZE = PGSIZE * 4,
          .DEADLINE = SOFT,
          .PERIOD = SYSTEM_TIME_INFINITE_VAL,
          .TIME_CAPACITY = SYSTEM_TIME_INFINITE_VAL,
      },
      {
          .NAME = "blackboard_read_check_1",
          .ENTRY_POINT = (SYSTEM_ADDRESS_TYPE)blackboard_read_check,
          .BASE_PRIORITY = 2,
          .STACK_SIZE = PGSIZE * 4,
          .DEADLINE = SOFT,
          .PERIOD = SYSTEM_TIME_INFINITE_VAL,
          .TIME_CAPACITY = SYSTEM_TIME_INFINITE_VAL,
      }};
  PROCESS_ID_TYPE proc_id[2];
  BLACKBOARD_ID_TYPE bb_id[2];
  check_e(CREATE_PROCESS(&attr[0], &proc_id[0], &ret));
  check_e(CREATE_PROCESS(&attr[1], &proc_id[1], &ret));
  check_e(START(proc_id[0], &ret));
  check_e(START(proc_id[1], &ret));
  check_e(CREATE_BLACKBOARD("blackboard_0", sizeof(uint32_t), &bb_id[0], &ret));
  check_e(CREATE_BLACKBOARD("blackboard_1", sizeof(uint32_t), &bb_id[1], &ret));
  check_e(SET_PARTITION_MODE(NORMAL, &ret));
  fatal("unreachable code\n");
}

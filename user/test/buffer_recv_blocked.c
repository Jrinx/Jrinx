#include <layouts.h>
#include <user/lib/debug.h>
#include <user/lib/logger.h>
#include <user/sys/services.h>
#include <user/sys/syscalls.h>

void buffer_send_check(void) {
  BUFFER_ID_TYPE buf_id;
  check_e(GET_BUFFER_ID("buffer", &buf_id, &ret));
  for (uint32_t i = 0; i < 5; i++) {
    info("send %u (len=%lu) into buffer %lu\n", i, sizeof(uint32_t), buf_id);
    check_e(SEND_BUFFER(buf_id, &i, sizeof(uint32_t), SYSTEM_TIME_INFINITE_VAL, &ret));
  }
  STOP_SELF();
}

void buffer_recv_check(void) {
  BUFFER_ID_TYPE buf_id;
  check_e(GET_BUFFER_ID("buffer", &buf_id, &ret));
  uint32_t data;
  msg_size_t len;
  for (uint32_t i = 0; i < 5; i++) {
    check_e(RECEIVE_BUFFER(buf_id, SYSTEM_TIME_INFINITE_VAL, &data, &len, &ret));
    info("recv %u (len=%lu) from buffer %lu\n", data, len, buf_id);
  }
  RETURN_CODE_TYPE ret;
  info("recv ... (timed out expected)\n");
  RECEIVE_BUFFER(buf_id, SYS_TIME_SECOND, &data, &len, &ret);
  if (ret != TIMED_OUT) {
    fatal("unexpected return code: %d\n", ret);
  } else {
    info("timed out\n");
  }
  BUFFER_STATUS_TYPE status;
  check_e(GET_BUFFER_STATUS(buf_id, &status, &ret));
  info("buffer status: nb=%lu/%lu, max_msg_size=%lu, nb_waiting_proc=%lu\n", status.NB_MESSAGE,
       status.MAX_NB_MESSAGE, status.MAX_MESSAGE_SIZE, status.WAITING_PROCESSES);
  halt("done\n");
}

void main(void) {
  PROCESS_ATTRIBUTE_TYPE attr[2] = {{
                                        .NAME = "buffer_send_check",
                                        .ENTRY_POINT = (SYSTEM_ADDRESS_TYPE)buffer_send_check,
                                        .BASE_PRIORITY = 1,
                                        .STACK_SIZE = PGSIZE * 4,
                                        .DEADLINE = SOFT,
                                        .PERIOD = SYSTEM_TIME_INFINITE_VAL,
                                        .TIME_CAPACITY = SYSTEM_TIME_INFINITE_VAL,
                                    },
                                    {
                                        .NAME = "buffer_recv_check",
                                        .ENTRY_POINT = (SYSTEM_ADDRESS_TYPE)buffer_recv_check,
                                        .BASE_PRIORITY = 2,
                                        .STACK_SIZE = PGSIZE * 4,
                                        .DEADLINE = SOFT,
                                        .PERIOD = SYSTEM_TIME_INFINITE_VAL,
                                        .TIME_CAPACITY = SYSTEM_TIME_INFINITE_VAL,
                                    }};
  PROCESS_ID_TYPE proc_id[2];
  BUFFER_ID_TYPE buf_id;
  check_e(CREATE_PROCESS(&attr[0], &proc_id[0], &ret));
  check_e(CREATE_PROCESS(&attr[1], &proc_id[1], &ret));
  check_e(START(proc_id[0], &ret));
  check_e(START(proc_id[1], &ret));
  check_e(CREATE_BUFFER("buffer", sizeof(uint32_t), 5, FIFO, &buf_id, &ret));
  check_e(SET_PARTITION_MODE(NORMAL, &ret));
  fatal("unreachable code\n");
}

#include <layouts.h>
#include <user/lib/debug.h>
#include <user/lib/logger.h>
#include <user/sys/services.h>
#include <user/sys/syscalls.h>

void player1(void) {
  info("I am player 1, and let me serve\n");
  uint32_t ball = 0;
  size_t ball_size;
  BUFFER_ID_TYPE buf_id;
  check_e(GET_BUFFER_ID("table", &buf_id, &ret));
  RETURN_CODE_TYPE ret;
  for (size_t i = 1; i <= 4; i++) {
    info("(round=%lu) send ball %u\n", i, ball);
    check_e(SEND_BUFFER(buf_id, &ball, sizeof(uint32_t), 0, &ret));
    info("(round=%lu) wait ball with timeout 2 s...\n", i);
    SUSPEND_SELF(2 * SYS_TIME_SECOND, &ret);
    if (ret == NO_ERROR) {
      RECEIVE_BUFFER(buf_id, 2 * SYS_TIME_SECOND, &ball, &ball_size, &ret);
      if (ret == NO_ERROR) {
        info("(round=%lu) got ball %u\n", i, ball);
      } else if (ret == TIMED_OUT) {
        info("(round=%lu) recv timed out\n", i);
      } else {
        fatal("(round=%lu) unexpected return code: %d\n", i, ret);
      }
    } else if (ret == TIMED_OUT) {
      info("(round=%lu) suspend timed out\n", i);
    } else {
      fatal("(round=%lu) unexpected return code: %d\n", i, ret);
    }
    ball++;
  }
  halt("game over\n");
}

void player2(void) {
  info("I am player 2\n");
  uint32_t ball;
  size_t ball_size;
  PROCESS_ID_TYPE that;
  BUFFER_ID_TYPE buf_id;
  check_e(GET_BUFFER_ID("table", &buf_id, &ret));
  check_e(GET_PROCESS_ID("player1", &that, &ret));
  RETURN_CODE_TYPE ret;

  // round 1
  info("(round=1) wait ball ...\n");
  check_e(RECEIVE_BUFFER(buf_id, 0, &ball, &ball_size, &ret));
  info("(round=1) got ball %u\n", ball);
  ball++;
  info("(round=1) send ball %u\n", ball);
  check_e(SEND_BUFFER(buf_id, &ball, sizeof(uint32_t), 0, &ret));
  check_e(RESUME(that, &ret));

  // round 2
  info("(round=2) wait ball ...\n");
  check_e(RECEIVE_BUFFER(buf_id, 0, &ball, &ball_size, &ret));
  info("(round=2) got ball %u\n", ball);
  info("(round=2) oh, ball lost\n");
  SUSPEND_SELF(2 * SYS_TIME_SECOND, &ret);
  if (ret != TIMED_OUT) {
    fatal("unexpected return code: %d\n", ret);
  }

  // round 3
  info("(round=3) wait ball ...\n");
  check_e(RECEIVE_BUFFER(buf_id, 0, &ball, &ball_size, &ret));
  info("(round=3) got ball %u\n", ball);
  info("(round=3) oh, ball sent but out of table\n");
  check_e(RESUME(that, &ret));

  // round 4
  info("(round=4) wait ball with timeout 4 s...\n");
  check_e(RECEIVE_BUFFER(buf_id, 4 * SYS_TIME_SECOND, &ball, &ball_size, &ret));
  info("(round=4) got ball %u\n", ball);
  ball++;
  info("(round=4) send ball %u\n", ball);
  check_e(SEND_BUFFER(buf_id, &ball, sizeof(uint32_t), 0, &ret));
  check_e(RESUME(that, &ret));

  STOP_SELF();
}

void main(void) {
  PROCESS_ATTRIBUTE_TYPE attr[2] = {{
                                        .NAME = "player1",
                                        .ENTRY_POINT = (SYSTEM_ADDRESS_TYPE)player1,
                                        .BASE_PRIORITY = 2,
                                        .STACK_SIZE = PGSIZE * 4,
                                        .DEADLINE = SOFT,
                                        .PERIOD = SYSTEM_TIME_INFINITE_VAL,
                                        .TIME_CAPACITY = SYSTEM_TIME_INFINITE_VAL,
                                    },
                                    {
                                        .NAME = "player2",
                                        .ENTRY_POINT = (SYSTEM_ADDRESS_TYPE)player2,
                                        .BASE_PRIORITY = 1,
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
  check_e(CREATE_BUFFER("table", sizeof(uint32_t), 1, FIFO, &buf_id, &ret));
  check_e(SET_PARTITION_MODE(NORMAL, &ret));
  fatal("unreachable code\n");
}

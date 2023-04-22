#include <layouts.h>
#include <user/lib/debug.h>
#include <user/lib/logger.h>
#include <user/sys/services.h>
#include <user/sys/syscalls.h>

void timed_wait_check(void) {
  PROCESS_ID_TYPE this, main;
  check_e(GET_MY_ID(&this, &ret));
  check_e(GET_PROCESS_ID("timed_wait_check_0", &main, &ret));
  if (this == main) {
    for (size_t i = 0; i < 6; i++) {
      info("this is %lu's (main's) turn\n", this);
      check_e(TIMED_WAIT(SYS_TIME_SECOND, &ret));
    }
    halt("done\n");
  } else {
    for (size_t i = 0; i < 6; i++) {
      info("this is %lu's turn\n", this);
      check_e(TIMED_WAIT(0, &ret));
    }
    STOP_SELF();
  }
}

void main(void) {
  PROCESS_ATTRIBUTE_TYPE attr[3] = {{
                                        .NAME = "timed_wait_check_0",
                                        .ENTRY_POINT = (SYSTEM_ADDRESS_TYPE)timed_wait_check,
                                        .BASE_PRIORITY = 1,
                                        .STACK_SIZE = PGSIZE * 4,
                                        .DEADLINE = SOFT,
                                        .PERIOD = SYSTEM_TIME_INFINITE_VAL,
                                        .TIME_CAPACITY = SYSTEM_TIME_INFINITE_VAL,
                                    },
                                    {
                                        .NAME = "timed_wait_check_1",
                                        .ENTRY_POINT = (SYSTEM_ADDRESS_TYPE)timed_wait_check,
                                        .BASE_PRIORITY = 1,
                                        .STACK_SIZE = PGSIZE * 4,
                                        .DEADLINE = SOFT,
                                        .PERIOD = SYSTEM_TIME_INFINITE_VAL,
                                        .TIME_CAPACITY = SYSTEM_TIME_INFINITE_VAL,
                                    },
                                    {
                                        .NAME = "timed_wait_check_2",
                                        .ENTRY_POINT = (SYSTEM_ADDRESS_TYPE)timed_wait_check,
                                        .BASE_PRIORITY = 1,
                                        .STACK_SIZE = PGSIZE * 4,
                                        .DEADLINE = SOFT,
                                        .PERIOD = SYSTEM_TIME_INFINITE_VAL,
                                        .TIME_CAPACITY = SYSTEM_TIME_INFINITE_VAL,
                                    }};
  PROCESS_ID_TYPE proc_id[3];
  check_e(CREATE_PROCESS(&attr[0], &proc_id[0], &ret));
  check_e(CREATE_PROCESS(&attr[1], &proc_id[1], &ret));
  check_e(CREATE_PROCESS(&attr[2], &proc_id[2], &ret));
  check_e(START(proc_id[0], &ret));
  check_e(START(proc_id[1], &ret));
  check_e(START(proc_id[2], &ret));
  check_e(SET_PARTITION_MODE(NORMAL, &ret));
  fatal("unreachable code\n");
}

#include <layouts.h>
#include <user/lib/debug.h>
#include <user/lib/logger.h>
#include <user/sys/services.h>
#include <user/sys/syscalls.h>

static void yield(PROCESS_ID_TYPE fr, PROCESS_ID_TYPE to) {
  PROCESS_STATUS_TYPE fr_status = {
      .ATTRIBUTES =
          {
              .NAME = NULL,
          },
  };
  check_e(GET_PROCESS_STATUS(fr, &fr_status, &ret));
  check_e(SET_PRIORITY(to, fr_status.CURRENT_PRIORITY + 1, &ret));
}

void put_a(void) {
  PROCESS_ID_TYPE proc_id[2];
  check_e(GET_MY_ID(&proc_id[0], &ret));
  check_e(GET_PROCESS_ID("put-b", &proc_id[1], &ret));
  info("put 'A' to console\n");
  while (1) {
    yield(proc_id[0], proc_id[1]);
    sys_cons_write_char('A');
  }
}

void put_b(void) {
  PROCESS_ID_TYPE proc_id[2];
  check_e(GET_PROCESS_ID("put-a", &proc_id[0], &ret));
  check_e(GET_MY_ID(&proc_id[1], &ret));
  info("put 'B' to console\n");
  while (1) {
    yield(proc_id[1], proc_id[0]);
    sys_cons_write_char('B');
  }
}

void main(void) {
  PROCESS_ATTRIBUTE_TYPE attr[2] = {{
                                        .NAME = "put-a",
                                        .ENTRY_POINT = (SYSTEM_ADDRESS_TYPE)put_a,
                                        .BASE_PRIORITY = 2,
                                        .STACK_SIZE = PGSIZE * 4,
                                        .DEADLINE = SOFT,
                                        .PERIOD = SYSTEM_TIME_INFINITE_VAL,
                                        .TIME_CAPACITY = SYSTEM_TIME_INFINITE_VAL,
                                    },
                                    {
                                        .NAME = "put-b",
                                        .ENTRY_POINT = (SYSTEM_ADDRESS_TYPE)put_b,
                                        .BASE_PRIORITY = 1,
                                        .STACK_SIZE = PGSIZE * 4,
                                        .DEADLINE = SOFT,
                                        .PERIOD = SYSTEM_TIME_INFINITE_VAL,
                                        .TIME_CAPACITY = SYSTEM_TIME_INFINITE_VAL,
                                    }};
  PROCESS_ID_TYPE proc_id[2];
  check_e(CREATE_PROCESS(&attr[0], &proc_id[0], &ret));
  check_e(CREATE_PROCESS(&attr[1], &proc_id[1], &ret));
  check_e(START(proc_id[0], &ret));
  check_e(START(proc_id[1], &ret));
  check_e(SET_PARTITION_MODE(NORMAL, &ret));
  fatal("unreachable code\n");
}

#include <layouts.h>
#include <user/lib/debug.h>
#include <user/lib/logger.h>
#include <user/sys/services.h>
#include <user/sys/syscalls.h>

void set_priority_check(void) {
  PROCESS_STATUS_TYPE this_status = {
      .ATTRIBUTES =
          {
              .NAME = NULL,
          },
  };
  PROCESS_ID_TYPE proc_id[2];
  check_e(GET_PROCESS_ID("set_priority_check_0", &proc_id[0], &ret));
  check_e(GET_PROCESS_ID("set_priority_check_1", &proc_id[1], &ret));
  PROCESS_ID_TYPE this, that;
  check_e(GET_MY_ID(&this, &ret));
  that = proc_id[!!(this == proc_id[0])];
  if (this == proc_id[0]) {
    for (size_t i = 0; i <= 3; i++) {
      check_e(GET_PROCESS_STATUS(this, &this_status, &ret));
      info("set %lu's priority to %lu\n", that, this_status.CURRENT_PRIORITY + 1);
      check_e(SET_PRIORITY(that, this_status.CURRENT_PRIORITY + 1, &ret));
    }
    halt("done\n");
  } else {
    while (1) {
      check_e(GET_PROCESS_STATUS(this, &this_status, &ret));
      info("reset %lu's priority\n", this);
      check_e(SET_PRIORITY(this, this_status.ATTRIBUTES.BASE_PRIORITY, &ret));
    }
  }
}

void main(void) {
  PROCESS_ATTRIBUTE_TYPE attr[2] = {{
                                        .NAME = "set_priority_check_0",
                                        .ENTRY_POINT = (SYSTEM_ADDRESS_TYPE)set_priority_check,
                                        .BASE_PRIORITY = 2,
                                        .STACK_SIZE = PGSIZE * 4,
                                        .DEADLINE = SOFT,
                                        .PERIOD = SYSTEM_TIME_INFINITE_VAL,
                                        .TIME_CAPACITY = SYSTEM_TIME_INFINITE_VAL,
                                    },
                                    {
                                        .NAME = "set_priority_check_1",
                                        .ENTRY_POINT = (SYSTEM_ADDRESS_TYPE)set_priority_check,
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

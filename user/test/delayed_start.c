#include <layouts.h>
#include <user/lib/debug.h>
#include <user/lib/logger.h>
#include <user/sys/services.h>
#include <user/sys/syscalls.h>

void delayed_start_check(void) {
  static size_t cnt = 0;
  cnt++;
  PROCESS_ID_TYPE proc_id[2];
  check_e(GET_PROCESS_ID("delayed_start_check_0", &proc_id[0], &ret));
  check_e(GET_PROCESS_ID("delayed_start_check_1", &proc_id[1], &ret));
  PROCESS_ID_TYPE this, that;
  check_e(GET_MY_ID(&this, &ret));
  that = proc_id[!!(this == proc_id[0])];
  info("%lu started, stop self and delayed start %lu\n", this, that);
  check_e(DELAYED_START(that, 1 * SYS_TIME_SECOND, &ret));
  if (cnt >= 4) {
    halt("done\n");
  }
  STOP_SELF();
}

void main(void) {
  PROCESS_ATTRIBUTE_TYPE attr[2] = {{
                                        .NAME = "delayed_start_check_0",
                                        .ENTRY_POINT = (SYSTEM_ADDRESS_TYPE)delayed_start_check,
                                        .BASE_PRIORITY = 1,
                                        .STACK_SIZE = PGSIZE * 4,
                                        .DEADLINE = SOFT,
                                        .PERIOD = SYSTEM_TIME_INFINITE_VAL,
                                        .TIME_CAPACITY = SYSTEM_TIME_INFINITE_VAL,
                                    },
                                    {
                                        .NAME = "delayed_start_check_1",
                                        .ENTRY_POINT = (SYSTEM_ADDRESS_TYPE)delayed_start_check,
                                        .BASE_PRIORITY = 1,
                                        .STACK_SIZE = PGSIZE * 4,
                                        .DEADLINE = SOFT,
                                        .PERIOD = SYSTEM_TIME_INFINITE_VAL,
                                        .TIME_CAPACITY = SYSTEM_TIME_INFINITE_VAL,
                                    }};
  PROCESS_ID_TYPE proc_id[2];
  check_e(CREATE_PROCESS(&attr[0], &proc_id[0], &ret));
  check_e(CREATE_PROCESS(&attr[1], &proc_id[1], &ret));
  check_e(START(proc_id[1], &ret));
  check_e(SET_PARTITION_MODE(NORMAL, &ret));
  fatal("unreachable code\n");
}

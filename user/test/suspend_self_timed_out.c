#include <layouts.h>
#include <user/lib/debug.h>
#include <user/lib/logger.h>
#include <user/sys/services.h>
#include <user/sys/syscalls.h>

void suspend_self_timed_out_check(void) {
  const SYSTEM_TIME_TYPE interval = SYS_TIME_SECOND;
  RETURN_CODE_TYPE ret;
  for (size_t i = 0; i < 3; i++) {
    SYSTEM_TIME_TYPE tick1, tick2;
    check_e(GET_TIME(&tick1, &ret));
    SUSPEND_SELF(interval, &ret);
    check_e(GET_TIME(&tick2, &ret));
    if (ret != TIMED_OUT) {
      fatal("unexpected return code: %d\n", ret);
    }
    if (tick2 - tick1 < interval) {
      fatal("too short\n");
    }
    info("timed out\n");
  }
  halt("done\n");
}

void main(void) {
  PROCESS_ATTRIBUTE_TYPE attr = {
      .NAME = "suspend_self_timed_out_check",
      .ENTRY_POINT = (SYSTEM_ADDRESS_TYPE)suspend_self_timed_out_check,
      .BASE_PRIORITY = 1,
      .STACK_SIZE = PGSIZE * 4,
      .DEADLINE = SOFT,
      .PERIOD = SYSTEM_TIME_INFINITE_VAL,
      .TIME_CAPACITY = SYSTEM_TIME_INFINITE_VAL,
  };
  PROCESS_ID_TYPE proc_id;
  check_e(CREATE_PROCESS(&attr, &proc_id, &ret));
  check_e(START(proc_id, &ret));
  check_e(SET_PARTITION_MODE(NORMAL, &ret));
  fatal("unreachable code\n");
}

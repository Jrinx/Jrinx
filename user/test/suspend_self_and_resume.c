#include <layouts.h>
#include <user/lib/debug.h>
#include <user/lib/logger.h>
#include <user/sys/services.h>
#include <user/sys/syscalls.h>

void suspend_self_check(void) {
  const SYSTEM_TIME_TYPE interval = 2 * SYS_TIME_SECOND;
  for (size_t i = 0; i < 3; i++) {
    SYSTEM_TIME_TYPE tick1, tick2;
    check_e(GET_TIME(&tick1, &ret));
    check_e(SUSPEND_SELF(interval, &ret));
    check_e(GET_TIME(&tick2, &ret));
    if (tick2 - tick1 >= interval) {
      fatal("too long\n");
    }
  }
  halt("done\n");
}

void resume_check(void) {
  PROCESS_ID_TYPE tgt;
  check_e(GET_PROCESS_ID("suspend_self_check", &tgt, &ret));
  while (1) {
    info("resume %lu\n", tgt);
    check_e(RESUME(tgt, &ret));
  }
}

void main(void) {
  PROCESS_ATTRIBUTE_TYPE attr[2] = {{
                                        .NAME = "suspend_self_check",
                                        .ENTRY_POINT = (SYSTEM_ADDRESS_TYPE)suspend_self_check,
                                        .BASE_PRIORITY = 2,
                                        .STACK_SIZE = PGSIZE * 4,
                                        .DEADLINE = SOFT,
                                        .PERIOD = SYSTEM_TIME_INFINITE_VAL,
                                        .TIME_CAPACITY = SYSTEM_TIME_INFINITE_VAL,
                                    },
                                    {
                                        .NAME = "resume_check",
                                        .ENTRY_POINT = (SYSTEM_ADDRESS_TYPE)resume_check,
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

#include <layouts.h>
#include <user/lib/debug.h>
#include <user/lib/logger.h>
#include <user/sys/services.h>
#include <user/sys/syscalls.h>

void put_a(void) {
  RETURN_CODE_TYPE ret;
  info("put 'A' to console\n");
  for (size_t i = 0; i < 4; i++) {
    info("(%lu) A\n", i);
    SUSPEND_SELF(1500 * SYS_TIME_MILLISECOND, &ret);
  }
  halt("done\n");
}

void main(void) {
  PROCESS_ATTRIBUTE_TYPE attr = {
      .NAME = "put-a",
      .ENTRY_POINT = (SYSTEM_ADDRESS_TYPE)put_a,
      .BASE_PRIORITY = 2,
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

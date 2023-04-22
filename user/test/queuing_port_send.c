#include <layouts.h>
#include <user/lib/debug.h>
#include <user/lib/logger.h>
#include <user/sys/services.h>

void queuing_port_send_check(void) {
  QUEUING_PORT_ID_TYPE port_id;
  check_e(GET_QUEUING_PORT_ID("value_sq", &port_id, &ret));
  uint32_t fibo[2] = {0, 1};
  while (1) {
    check_e(SEND_QUEUING_MESSAGE(port_id, &fibo[0], 4, SYSTEM_TIME_INFINITE_VAL, &ret));
    uint32_t tmp = fibo[0] + fibo[1];
    fibo[0] = fibo[1];
    fibo[1] = tmp;
  }
}

void main(void) {
  PROCESS_ATTRIBUTE_TYPE attr = {
      .NAME = "queuing_port_send_check",
      .ENTRY_POINT = (SYSTEM_ADDRESS_TYPE)queuing_port_send_check,
      .BASE_PRIORITY = 1,
      .STACK_SIZE = 4 * PGSIZE,
      .DEADLINE = SOFT,
      .PERIOD = SYSTEM_TIME_INFINITE_VAL,
      .TIME_CAPACITY = SYSTEM_TIME_INFINITE_VAL,
  };
  PROCESS_ID_TYPE proc_id;
  check_e(CREATE_PROCESS(&attr, &proc_id, &ret));
  check_e(START(proc_id, &ret));

  QUEUING_PORT_ID_TYPE port_id;
  check_e(CREATE_QUEUING_PORT("value_sq", 4, 3, SOURCE, FIFO, &port_id, &ret));

  check_e(SET_PARTITION_MODE(NORMAL, &ret));
}

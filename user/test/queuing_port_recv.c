#include <layouts.h>
#include <user/lib/debug.h>
#include <user/lib/logger.h>
#include <user/sys/services.h>

void queuing_port_recv_check(void) {
  QUEUING_PORT_ID_TYPE port_id;
  check_e(GET_QUEUING_PORT_ID("value_dq", &port_id, &ret));
  for (size_t i = 0; i < 10; i++) {
    uint32_t value;
    size_t len;
    check_e(RECEIVE_QUEUING_MESSAGE(port_id, 4 * SYS_TIME_SECOND, &value, &len, &ret));
    info("fibo[%lu] = %u\n", i, value);
  }
  QUEUING_PORT_STATUS_TYPE status;
  check_e(GET_QUEUING_PORT_STATUS(port_id, &status, &ret));
  info("queuing port status: nb=%lu/%lu, max_msg_size=%lu, nb_waiting_proc=%lu\n",
       status.NB_MESSAGE, status.MAX_NB_MESSAGE, status.MAX_MESSAGE_SIZE,
       status.WAITING_PROCESSES);
  halt("done\n");
}

void main(void) {
  PROCESS_ATTRIBUTE_TYPE attr = {
      .NAME = "queuing_port_recv_check",
      .ENTRY_POINT = (SYSTEM_ADDRESS_TYPE)queuing_port_recv_check,
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
  check_e(CREATE_QUEUING_PORT("value_dq", 4, 3, DESTINATION, FIFO, &port_id, &ret));

  check_e(SET_PARTITION_MODE(NORMAL, &ret));
}

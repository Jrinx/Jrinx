#include <sysno.h>
#include <user/lib/debug.h>
#include <user/lib/logger.h>
#include <user/sys/services.h>

extern void main(void) __noreturn;

__noreturn void _start(void) {
  PARTITION_STATUS_TYPE part_init_status;
  check_e(GET_PARTITION_STATUS(&part_init_status, &ret));
  logger_init(part_init_status.IDENTIFIER);
  main();
}

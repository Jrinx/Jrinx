#include <sysno.h>
#include <user/lib/debug.h>
#include <user/lib/logger.h>
#include <user/sys/services.h>

extern void main(void) __attribute__((noreturn));

__attribute__((noreturn)) void _runtime(void) {
  PARTITION_STATUS_TYPE part_init_status;
  check_e(GET_PARTITION_STATUS(&part_init_status, &ret));
  logger_init(part_init_status.IDENTIFIER);
  info("partition %lu init with status:\n", part_init_status.IDENTIFIER);
  info("- period: %ld us\n", part_init_status.PERIOD);
  info("- duration: %ld us\n", part_init_status.DURATION);
  info("- lock level: %lu\n", part_init_status.LOCK_LEVEL);
  info("- start cond: %u\n", part_init_status.START_CONDITION);
  info("- assigned cores: %lu\n", part_init_status.NUM_ASSIGNED_CORES);
  main();
}

#include <kern/drivers/cpus.h>
#include <kern/lib/boottime.h>
#include <kern/lib/regs.h>

sys_time_t boottime_get_now(void) {
  uint64_t time = csrr_time();
  if (cpus_get_timebase_freq() == 0) {
    return 0;
  }
  return time / (cpus_get_timebase_freq() / SYS_TIME_SECOND);
}

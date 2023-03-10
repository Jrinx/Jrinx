#include <kern/multitask/sched.h>
#include <kern/traps/traps.h>

void do_timer_int(struct context *context) {
  sched();
}

#include <kern/drivers/intc.h>
#include <kern/multitask/sched.h>
#include <kern/traps/timer.h>
#include <kern/traps/traps.h>

void do_timer_int(struct context *context) {
  time_event_action();
}

void do_external_int(struct context *context) {
  trap_callback_t callback;
  intc_get_handler(context->ctx_scause, &callback);
  if (callback.cb_func == NULL) {
    fatal("Unhandled external interrupt: %08lx\n", context->ctx_scause);
  }
  panic_e(cb_invoke(callback)(context->ctx_scause));
}

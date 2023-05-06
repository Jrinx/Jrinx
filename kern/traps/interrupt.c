#include <kern/drivers/intc.h>
#include <kern/lib/boottime.h>
#include <kern/multitask/sched.h>
#include <kern/traps/timer.h>
#include <kern/traps/traps.h>

sys_time_t *debug_timer_claim_expected_time;
sys_time_t *debug_timer_claim_time;
sys_time_t *debug_timer_complete_time;

void do_timer_int(struct context *context) {
  extern int args_debug_tick_num;
  static int tick_num = 0;
  if (args_debug_tick_num) {
    sys_time_t now = boottime_get_now();
    debug_timer_claim_time[tick_num] = now;
  }
  time_event_action();
  if (args_debug_tick_num) {
    sys_time_t now = boottime_get_now();
    debug_timer_complete_time[tick_num] = now;
    tick_num++;
    if (tick_num >= args_debug_tick_num) {
      sys_time_t diff_comp_act_time_tot = 0;
      sys_time_t diff_act_time_exp_time_tot = 0;
      for (size_t i = 0; i < args_debug_tick_num; i++) {
        // calculate the difference between actual claim time and complete time
        diff_comp_act_time_tot += debug_timer_complete_time[i] - debug_timer_claim_time[i];
        // calculate the difference between expected claim time and actual claim time
        diff_act_time_exp_time_tot +=
            debug_timer_claim_time[i] - debug_timer_claim_expected_time[i];
      }
      halt("tick (total: %d) diff<T_act_claim, T_exp_claim>: %ld us, diff<T_comp, "
           "T_act_time>: %ld us\n",
           args_debug_tick_num, diff_act_time_exp_time_tot, diff_comp_act_time_tot);
    }
  }
}

void do_external_int(struct context *context) {
  rv64_si sip;
  do {
    trap_callback_t callback;
    intc_get_handler(context->ctx_scause, &callback);
    if (callback.cb_func == NULL) {
      fatal("Unhandled external interrupt: %08lx\n", context->ctx_scause);
    }
    panic_e(cb_invoke(callback)(context->ctx_scause));
    sip.val = csrr_sip();
  } while (sip.bits.sei);
}

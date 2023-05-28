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
      sys_time_t diff_comp_act_time_max = 0;
      sys_time_t diff_act_time_exp_time_max = 0;
      for (size_t i = 0; i < args_debug_tick_num; i++) {
        // calculate the difference between actual claim time and complete time
        sys_time_t diff1 = debug_timer_complete_time[i] - debug_timer_claim_time[i];
        diff_comp_act_time_tot += diff1;
        if (diff1 > diff_comp_act_time_max) {
          diff_comp_act_time_max = diff1;
        }
        // calculate the difference between expected claim time and actual claim time
        sys_time_t diff2 = debug_timer_claim_time[i] - debug_timer_claim_expected_time[i];
        diff_act_time_exp_time_tot += diff2;
        if (diff2 > diff_act_time_exp_time_max) {
          diff_act_time_exp_time_max = diff2;
        }
      }
      info("time tick total: %d\n", args_debug_tick_num);
      info("diff<T_act_claim, T_exp_claim>: {tot=%ld us, max=%ld us}\n",
           diff_act_time_exp_time_tot, diff_act_time_exp_time_max);
      info("diff<T_comp, T_act_time>: {tot=%ld us, max=%ld us}\n", diff_comp_act_time_tot,
           diff_comp_act_time_max);
      halt("real-time performance measurement done!\n");
    }
  }
}

void do_external_int(struct context *context) {
  rv64_si sip;
  trap_callback_t callback;
  intc_get_handler(context->ctx_scause, &callback);
  if (callback.cb_func == NULL) {
    fatal("Unhandled external interrupt: %08lx\n", context->ctx_scause);
  }
  do {
    panic_e(cb_invoke(callback)(context->ctx_scause));
    sip.val = csrr_sip();
  } while (sip.bits.sei);
}

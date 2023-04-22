#include <kern/chan/queuing.h>
#include <kern/comm/buffer.h>
#include <kern/drivers/serialport.h>
#include <kern/lib/boottime.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/mm/kalloc.h>
#include <kern/multitask/sched.h>
#include <kern/traps/timer.h>
#include <kern/traps/traps.h>
#include <layouts.h>
#include <lib/string.h>
#include <sysno.h>
#include <types.h>

void do_cons_write_char(int ch) {
  serial_blocked_putc(ch);
}

int do_cons_read_char(void) {
  return serial_blocked_getc();
}

void do_cons_write_buf(const char *buf, size_t len) {
  for (size_t i = 0; i < len; i++) {
    serial_blocked_putc(buf[i]);
  }
}

void do_halt(void) {
  halt("shutdown from syscall\n");
}

ret_code_t do_get_partition_status(PARTITION_STATUS_TYPE *partition_status) {
  const struct part *part = sched_cur_part();
  partition_status->PERIOD = part->pa_period;
  partition_status->DURATION = part->pa_duration;
  partition_status->IDENTIFIER = part->pa_id;
  partition_status->LOCK_LEVEL = part->pa_lock_level;
  partition_status->OPERATING_MODE = part->pa_op_mode;
  partition_status->START_CONDITION = part->pa_start_cond;
  partition_status->NUM_ASSIGNED_CORES = part->pa_num_cores;
  return NO_ERROR;
}

ret_code_t do_set_partition_mode(uintmax_t operating_mode) {
  struct part *part = sched_cur_part();
  if (operating_mode == NORMAL && part->pa_op_mode == NORMAL) {
    return NO_ACTION;
  }
  if (operating_mode == WARM_START && part->pa_op_mode == COLD_START) {
    return INVALID_MODE;
  }
  part->pa_op_mode = operating_mode;
  switch (operating_mode) {
  case IDLE:
    // TODO: shutdown the partition
    break;
  case WARM_START:
  case COLD_START:
    // TODO: inhibit process scheduling, switch back to init mode
    break;
  case NORMAL:
    // TODO:
    // 1. set to READY all previous started (not delayed) aperiodic processes, unless the
    //    process was suspended
    // 2. set release point of all previous delayed aperiodic processes to the system clock time
    //    plus their delay times
    // 3. set first release point of all previously started (not delayed) periodic processes to
    //    the partition's next period processing start
    // 4. set first release point of all previously delayed started periodic processes to the
    //    partition's next periodic processing start plus their delay times
    // 5. calculate the DEADLINE_TIME of all non-dormant processes in the partition
    // 6. set the partition's lock level to zero
    // 7. schedule the partition's processes
    struct proc *proc;
    LINKED_NODE_ITER (part->pa_proc_list.l_first, proc, pr_sched_link) {
      // TODO: should follow std above.
      if (proc->pr_state == WAITING && proc->pr_waiting_reason == PREVIOUS_STARTED) {
        proc->pr_state = READY;
      }
    }
    sched_cur_proc()->pr_state = DORMANT;
    sched_proc_give_up(0);
    break;
  default:
    return INVALID_PARAM;
    break;
  }
  return NO_ERROR;
}

ret_code_t do_get_process_id(proc_name_t process_name, proc_id_t *process_id) {
  struct proc *proc = part_get_proc_by_name(sched_cur_part(), process_name);
  if (proc == NULL) {
    return INVALID_CONFIG;
  }
  *process_id = proc->pr_id;
  return NO_ERROR;
}

ret_code_t do_get_process_status(proc_id_t process_id, PROCESS_STATUS_TYPE *process_status) {
  struct proc *proc = proc_from_id(process_id);
  if (proc == NULL || proc->pr_part_id != sched_cur_part()->pa_id) {
    return INVALID_PARAM;
  }
  process_status->DEADLINE_TIME = proc->pr_deadline_time;
  // TODO: check if proc owns preemption lock
  process_status->CURRENT_PRIORITY = proc->pr_cur_pri;
  process_status->PROCESS_STATE = proc->pr_state;
  process_status->ATTRIBUTES.PERIOD = proc->pr_period;
  process_status->ATTRIBUTES.TIME_CAPACITY = proc->pr_time_cap;
  process_status->ATTRIBUTES.ENTRY_POINT = proc->pr_entrypoint;
  process_status->ATTRIBUTES.STACK_SIZE = proc->pr_ustacksize;
  process_status->ATTRIBUTES.BASE_PRIORITY = proc->pr_base_pri;
  process_status->ATTRIBUTES.DEADLINE = proc->pr_deadline;
  if (process_status->ATTRIBUTES.NAME != NULL) {
    strcpy(process_status->ATTRIBUTES.NAME, proc->pr_name);
  }
  return NO_ERROR;
}

ret_code_t do_create_process(PROCESS_ATTRIBUTE_TYPE *attributes, proc_id_t *process_id) {
  struct part *part = sched_cur_part();
  struct proc *proc = part_get_proc_by_name(sched_cur_part(), attributes->NAME);
  if (proc != NULL) {
    return NO_ACTION;
  }
  if (attributes->STACK_SIZE > USTKSIZEMAX) {
    return INVALID_CONFIG;
  }
  if (attributes->PERIOD < SYSTEM_TIME_INFINITE_VAL &&
      attributes->PERIOD % part->pa_period != 0) {
    return INVALID_CONFIG;
  }
  if (attributes->PERIOD < SYSTEM_TIME_INFINITE_VAL &&
      attributes->PERIOD < attributes->TIME_CAPACITY) {
    return INVALID_CONFIG;
  }
  if (part->pa_op_mode == NORMAL) {
    return INVALID_MODE;
  }
  catch_e(proc_alloc(part, &proc, attributes->NAME, attributes->PERIOD,
                     attributes->TIME_CAPACITY, attributes->ENTRY_POINT, attributes->STACK_SIZE,
                     attributes->BASE_PRIORITY, attributes->DEADLINE),
          { return INVALID_CONFIG; });
  *process_id = proc->pr_id;
  return NO_ERROR;
}

ret_code_t do_set_priority(proc_id_t process_id, priority_t priority) {
  struct proc *proc = proc_from_id(process_id);
  if (proc == NULL || proc->pr_part_id != sched_cur_part()->pa_id) {
    return INVALID_PARAM;
  }
  if (proc->pr_state == DORMANT) {
    return INVALID_MODE;
  }
  // TODO: check if proc owns mutex?
  proc->pr_cur_pri = priority;
  sched_proc_give_up(0);
  return NO_ERROR;
}

ret_code_t do_suspend_self(sys_time_t time_out) {
  struct proc *proc = sched_cur_proc();
  // TODO: check if proc owns mutex or is error handler process?
  if (proc_is_period(proc)) {
    return INVALID_MODE;
  }
  if (time_out == 0) {
    return NO_ERROR;
  }
  proc->pr_state = WAITING;
  if (time_out < SYSTEM_TIME_INFINITE_VAL) {
    sys_time_t wakeup_time = boottime_get_now() + time_out;
    time_event_alloc(proc, wakeup_time, TE_PROCESS_SUSPEND_TIMEOUT);
    proc->pr_waiting_reason = SUSPENDED_WITH_TIMEOUT;
    sched_proc_give_up(0);
    if (boottime_get_now() >= wakeup_time) {
      return TIMED_OUT;
    }
  } else {
    proc->pr_waiting_reason = SUSPENDED;
    sched_proc_give_up(0);
  }
  return NO_ERROR;
}

ret_code_t do_suspend(proc_id_t process_id) {
  struct proc *proc = proc_from_id(process_id);
  if (proc == NULL || proc->pr_part_id != sched_cur_part()->pa_id) {
    return INVALID_PARAM;
  }
  // TODO: check if proc owns mutex or is waiting on a mutex's queue
  if (proc->pr_state == DORMANT || proc->pr_state == FAULTED) {
    return INVALID_MODE;
  }
  if (proc_is_period(proc)) {
    return INVALID_MODE;
  }
  if (proc->pr_state == WAITING) {
    return NO_ACTION;
  }
  proc->pr_state = WAITING;
  proc->pr_waiting_reason = SUSPENDED;
  sched_proc_give_up(0);
  return NO_ERROR;
}

ret_code_t do_resume(proc_id_t process_id) {
  struct proc *proc = proc_from_id(process_id);
  if (proc == NULL || proc->pr_part_id != sched_cur_part()->pa_id) {
    return INVALID_PARAM;
  }
  if (proc->pr_state == DORMANT) {
    return INVALID_MODE;
  }
  if (proc_is_period(proc) && proc->pr_state != FAULTED) {
    return INVALID_MODE;
  }
  if (!(proc->pr_state == WAITING && (proc->pr_waiting_reason == SUSPENDED ||
                                      proc->pr_waiting_reason == SUSPENDED_WITH_TIMEOUT)) &&
      proc->pr_state != FAULTED) {
    return NO_ACTION;
  }
  if (proc->pr_state == WAITING && proc->pr_waiting_reason == SUSPENDED_WITH_TIMEOUT) {
    time_event_free(proc->pr_asso_timer);
  }
  // TODO: check if proc is waiting on a process queue or TIMED_WAIT time delay or DELAYED_START
  // time delay
  proc->pr_state = READY;
  sched_proc_give_up(0);
  return NO_ERROR;
}

void do_stop_self(void) {
  struct proc *proc = sched_cur_proc();
  // TODO: check if proc owns preemption lock mutex
  proc->pr_state = DORMANT;
  // TODO: prevent proc from causing a deadline overrun fault
  sched_proc_give_up(0);
}

ret_code_t do_stop(proc_id_t process_id) {
  struct proc *proc = proc_from_id(process_id);
  if (proc == NULL || proc->pr_part_id != sched_cur_part()->pa_id) {
    return INVALID_PARAM;
  }
  if (proc->pr_state == DORMANT) {
    return NO_ACTION;
  }
  // TODO: check if proc owns preemption lock mutex, release...
  proc->pr_state = DORMANT;
  // TODO: check if proc is waiting on a process queue, remove it from queue
  if (proc->pr_asso_timer != NULL) {
    time_event_free(proc->pr_asso_timer);
  }
  // TODO: prevent the process from causing any deadline overrun faults
  sched_proc_give_up(0);
  return NO_ERROR;
}

ret_code_t do_start(proc_id_t process_id) {
  ret_code_t do_delayed_start(proc_id_t process_id, sys_time_t delay_time);
  return do_delayed_start(process_id, 0);
}

ret_code_t do_delayed_start(proc_id_t process_id, sys_time_t delay_time) {
  struct proc *proc = proc_from_id(process_id);
  if (proc == NULL || proc->pr_part_id != sched_cur_part()->pa_id) {
    return INVALID_PARAM;
  }
  if (proc->pr_state != DORMANT) {
    return NO_ACTION;
  }
  if (delay_time == SYSTEM_TIME_INFINITE_VAL) {
    return INVALID_PARAM;
  }
  if (proc_is_period(proc) && delay_time >= proc->pr_period) {
    return INVALID_PARAM;
  }
  if (!proc_is_period(proc)) {
    proc->pr_cur_pri = proc->pr_base_pri;
    proc_reset(proc);
    if (sched_cur_part()->pa_op_mode == NORMAL) {
      if (delay_time == 0) {
        proc->pr_state = READY;
        proc->pr_deadline_time = boottime_get_now() + proc->pr_time_cap;
        // TODO: detect deadline exceeding
      } else {
        proc->pr_state = WAITING;
        proc->pr_waiting_reason = DELAYED_STARTED;
        time_event_alloc(proc, boottime_get_now() + delay_time, TE_PROCESS_DELAYED_START);
        proc->pr_deadline_time = boottime_get_now() + proc->pr_time_cap + delay_time;
        // TODO: detect deadline exceeding
        // TODO: init timer with duration delay_time
      }
      sched_proc_give_up(0);
    } else {
      proc->pr_state = WAITING;
      proc->pr_waiting_reason = PREVIOUS_STARTED;
    }
  } else {
    proc->pr_cur_pri = proc->pr_base_pri;
    proc_reset(proc);
    if (sched_cur_part()->pa_op_mode == NORMAL) {
      proc->pr_state = WAITING;
      proc->pr_waiting_reason = DELAYED_STARTED;
      // TODO: set the first release point of proc including delay_time
      // TODO: set DEADLINE_TIME to first release point + proc->pr_time_cap
    } else {
      proc->pr_state = WAITING;
      proc->pr_waiting_reason = PREVIOUS_STARTED;
    }
  }
  return NO_ERROR;
}

ret_code_t do_lock_preemption(lock_level_t *lock_level) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_unlock_preemption(lock_level_t *lock_level) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_get_my_id(proc_id_t *process_id) {
  struct proc *proc = sched_cur_proc();
  // TODO: check if proc has id, e.g. error handler process, return INVALID_MODE
  *process_id = proc->pr_id;
  return NO_ERROR;
}

ret_code_t do_initialize_process_core_affinity(proc_id_t process_id,
                                               proc_core_id_t processor_core_id) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_get_my_processor_core_id(proc_core_id_t *processor_core_id) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_get_my_index(proc_index_t *process_index) {
  struct proc *proc = sched_cur_proc();
  if (proc->pr_idx < 1) {
    return INVALID_MODE;
  }
  *process_index = proc->pr_idx;
  return NO_ERROR;
}

ret_code_t do_timed_wait(sys_time_t delay_time) {
  struct proc *proc = sched_cur_proc();
  // TODO: check if proc owns a mutex or is a error handler process, return INVALID_MODE
  if (delay_time == SYSTEM_TIME_INFINITE_VAL) {
    return INVALID_PARAM;
  }
  if (delay_time == 0) {
    sched_proc_give_up(1);
  } else {
    proc->pr_state = WAITING;
    proc->pr_waiting_reason = TIMED_WAIT_TIMEOUT;
    sys_time_t wakeup_time = boottime_get_now() + delay_time;
    time_event_alloc(proc, wakeup_time, TE_PROCESS_TIMED_WAIT_TIMEOUT);
    sched_proc_give_up(0);
  }
  return NO_ERROR;
}

ret_code_t do_periodic_wait(void) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_get_time(sys_time_t *system_time) {
  *system_time = boottime_get_now();
  return NO_ERROR;
}

ret_code_t do_replenish(sys_time_t budget_time) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_create_sampling_port(samp_port_name_t sampling_port_name,
                                   msg_size_t max_message_size, port_dir_t port_direction,
                                   sys_time_t refresh_period,
                                   samp_port_id_t *sampling_port_id) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_write_sampling_message(samp_port_id_t sampling_port_id, msg_addr_t message_addr,
                                     msg_size_t length) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_read_sampling_message(samp_port_id_t sampling_port_id, msg_addr_t message_addr,
                                    msg_size_t *length, validity_t *validity) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_get_sampling_port_id(samp_port_name_t sampling_port_name,
                                   samp_port_id_t *sampling_port_id) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_get_sampling_port_status(samp_port_id_t sampling_port_id,
                                       SAMPLING_PORT_STATUS_TYPE *sampling_port_status) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_create_queuing_port(que_port_name_t queuing_port_name,
                                  msg_size_t max_message_size, msg_range_t max_nb_message,
                                  port_dir_t port_direction, que_disc_t queuing_discipline,
                                  que_port_id_t *queuing_port_id) {
  struct part *part = sched_cur_part();
  if (!queuing_port_conf_validate(part, queuing_port_name, port_direction, max_message_size,
                                  max_nb_message)) {
    return INVALID_CONFIG;
  }
  if (queuing_port_from_name(queuing_port_name) != NULL) {
    return NO_ACTION;
  }
  if (part->pa_op_mode == NORMAL) {
    return INVALID_MODE;
  }
  struct queuing_port *port;
  catch_e(queuing_port_alloc(part, &port, queuing_port_name, port_direction, max_message_size,
                             max_nb_message, queuing_discipline),
          { return INVALID_CONFIG; });
  *queuing_port_id = port->qp_id;
  return NO_ERROR;
}

ret_code_t do_send_queuing_message(que_port_id_t queuing_port_id, msg_addr_t message_addr,
                                   msg_size_t length, sys_time_t time_out) {
  struct proc *proc = sched_cur_proc();
  struct queuing_port *port = queuing_port_from_id(queuing_port_id);
  if (port == NULL || port->qp_part_id != sched_cur_part()->pa_id) {
    return INVALID_PARAM;
  }
  if (length > port->qp_channel->ch_view.queuing.ch_max_msg_size) {
    return INVALID_CONFIG;
  }
  if (length == 0) {
    return INVALID_PARAM;
  }
  if (port->qp_dir != SOURCE) {
    return INVALID_MODE;
  }
  panic_e(lk_acquire(&port->qp_channel->ch_lock));
  if (queuing_port_is_full(port)) {
    if (time_out == 0) {
      panic_e(lk_release(&port->qp_channel->ch_lock));
      return NOT_AVAILABLE;
      // TODO: check if proc owns a mutex or is error handler
    } else {
      proc->pr_state = WAITING;
      queuing_port_add_waiting_proc(port, proc);
      sys_time_t wakeup_time = boottime_get_now() + time_out;
      struct te_proc_queuing_port *teqpp = NULL;
      if (time_out != SYSTEM_TIME_INFINITE_VAL) {
        proc->pr_waiting_reason = QUEUING_PORT_BLOCKED_WITH_TIMEOUT;
        teqpp = kalloc(sizeof(struct te_proc_queuing_port));
        teqpp->tepqp_proc = proc;
        teqpp->tepqp_port = port;
        time_event_alloc(teqpp, wakeup_time, TE_QUEUING_PORT_BLOCK_TIMEOUT);
      } else {
        proc->pr_waiting_reason = QUEUING_PORT_BLOCKED;
      }
      panic_e(lk_release(&port->qp_channel->ch_lock));
      sched_proc_give_up(0);
      if (time_out != SYSTEM_TIME_INFINITE_VAL) {
        kfree(teqpp);
        if (boottime_get_now() >= wakeup_time) {
          return TIMED_OUT;
        }
      }
    }
  } else {
    panic_e(lk_release(&port->qp_channel->ch_lock));
  }
  queuing_port_send(port, message_addr, length);
  panic_e(lk_acquire(&port->qp_channel->ch_lock));
  struct proc *to_wakeup = queuing_port_wakeup_waiting_proc(port);
  if (to_wakeup != NULL) {
    if (to_wakeup->pr_waiting_reason == QUEUING_PORT_BLOCKED_WITH_TIMEOUT) {
      time_event_free(to_wakeup->pr_asso_timer);
    }
    to_wakeup->pr_state = READY;
  }
  panic_e(lk_release(&port->qp_channel->ch_lock));
  return NO_ERROR;
}

ret_code_t do_receive_queuing_message(que_port_id_t queuing_port_id, sys_time_t time_out,
                                      msg_addr_t message_addr, msg_size_t *length) {
  struct proc *proc = sched_cur_proc();
  struct queuing_port *port = queuing_port_from_id(queuing_port_id);
  if (port == NULL || port->qp_part_id != sched_cur_part()->pa_id) {
    return INVALID_PARAM;
  }
  if (port->qp_dir != DESTINATION) {
    return INVALID_MODE;
  }
  panic_e(lk_acquire(&port->qp_channel->ch_lock));
  if (queuing_port_is_empty(port)) {
    if (time_out == 0) {
      panic_e(lk_release(&port->qp_channel->ch_lock));
      return NOT_AVAILABLE;
      // TODO: check if proc owns a mutex or is error handler
    } else {
      proc->pr_state = WAITING;
      queuing_port_add_waiting_proc(port, proc);
      sys_time_t wakeup_time = boottime_get_now() + time_out;
      struct te_proc_queuing_port *teqpp = NULL;
      if (time_out != SYSTEM_TIME_INFINITE_VAL) {
        proc->pr_waiting_reason = QUEUING_PORT_BLOCKED_WITH_TIMEOUT;
        teqpp = kalloc(sizeof(struct te_proc_queuing_port));
        teqpp->tepqp_proc = proc;
        teqpp->tepqp_port = port;
        time_event_alloc(teqpp, wakeup_time, TE_QUEUING_PORT_BLOCK_TIMEOUT);
      } else {
        proc->pr_waiting_reason = QUEUING_PORT_BLOCKED;
      }
      panic_e(lk_release(&port->qp_channel->ch_lock));
      sched_proc_give_up(0);
      if (time_out != SYSTEM_TIME_INFINITE_VAL) {
        kfree(teqpp);
        if (boottime_get_now() >= wakeup_time) {
          return TIMED_OUT;
        }
      }
    }
  } else {
    panic_e(lk_release(&port->qp_channel->ch_lock));
  }
  queuing_port_recv(port, message_addr, length);
  panic_e(lk_acquire(&port->qp_channel->ch_lock));
  struct proc *to_wakeup = queuing_port_wakeup_waiting_proc(port);
  if (to_wakeup != NULL) {
    if (to_wakeup->pr_waiting_reason == QUEUING_PORT_BLOCKED_WITH_TIMEOUT) {
      time_event_free(to_wakeup->pr_asso_timer);
    }
    to_wakeup->pr_state = READY;
  }
  panic_e(lk_release(&port->qp_channel->ch_lock));
  return NO_ERROR;
}

ret_code_t do_get_queuing_port_id(que_port_name_t queuing_port_name,
                                  que_port_id_t *queuing_port_id) {
  struct queuing_port *port = queuing_port_from_name(queuing_port_name);
  if (port == NULL || port->qp_part_id != sched_cur_part()->pa_id) {
    return INVALID_CONFIG;
  }
  *queuing_port_id = port->qp_id;
  return NO_ERROR;
}

ret_code_t do_get_queuing_port_status(que_port_id_t queuing_port_id,
                                      QUEUING_PORT_STATUS_TYPE *queuing_port_status) {
  struct queuing_port *port = queuing_port_from_id(queuing_port_id);
  if (port == NULL || port->qp_part_id != sched_cur_part()->pa_id) {
    return INVALID_PARAM;
  }
  queuing_port_status->MAX_MESSAGE_SIZE = port->qp_channel->ch_view.queuing.ch_max_msg_size;
  queuing_port_status->MAX_NB_MESSAGE = port->qp_channel->ch_view.queuing.ch_max_nb_msg;
  queuing_port_status->PORT_DIRECTION = port->qp_dir;
  queuing_port_status->NB_MESSAGE = port->qp_channel->ch_view.queuing.ch_nb_msg;
  queuing_port_status->WAITING_PROCESSES = queuing_port_get_wait_proc_nb(port);
  return NO_ERROR;
}

ret_code_t do_clear_queuing_port(que_port_id_t queuing_port_id) {
  struct queuing_port *port = queuing_port_from_id(queuing_port_id);
  if (port == NULL || port->qp_part_id != sched_cur_part()->pa_id) {
    return INVALID_PARAM;
  }
  if (port->qp_dir != DESTINATION) {
    return INVALID_MODE;
  }
  panic_e(lk_acquire(&port->qp_channel->ch_lock));
  if (queuing_port_is_full(port)) {
    struct proc *to_wakeup = queuing_port_wakeup_waiting_proc(port);
    if (to_wakeup != NULL) {
      if (to_wakeup->pr_waiting_reason == QUEUING_PORT_BLOCKED_WITH_TIMEOUT) {
        time_event_free(to_wakeup->pr_asso_timer);
      }
      to_wakeup->pr_state = READY;
    }
  }
  port->qp_channel->ch_view.queuing.ch_nb_msg = 0;
  port->qp_channel->ch_view.queuing.ch_off_b = 0;
  port->qp_channel->ch_view.queuing.ch_off_e =
      -(port->qp_channel->ch_view.queuing.ch_max_msg_size + sizeof(struct comm_msg));
  panic_e(lk_release(&port->qp_channel->ch_lock));
  return NO_ERROR;
}

ret_code_t do_create_buffer(buf_name_t buffer_name, msg_size_t max_message_size,
                            msg_range_t max_nb_message, que_disc_t queuing_discipline,
                            buf_id_t *buffer_id) {
  struct part *part = sched_cur_part();
  struct buffer *buf = part_get_buf_by_name(part, buffer_name);
  if (buf != NULL) {
    return NO_ACTION;
  }
  if (max_message_size == 0) {
    return INVALID_PARAM;
  }
  if (part->pa_comm_base + (max_message_size + sizeof(struct comm_msg)) * max_nb_message >=
      COMM_LIMT) {
    return INVALID_PARAM;
  }
  if (part->pa_op_mode == NORMAL) {
    return INVALID_MODE;
  }
  catch_e(buffer_alloc(part, &buf, buffer_name, max_message_size, max_nb_message,
                       queuing_discipline),
          { return INVALID_CONFIG; });
  *buffer_id = buf->buf_id;
  return NO_ERROR;
}

ret_code_t do_send_buffer(buf_id_t buffer_id, msg_addr_t message_addr, msg_size_t length,
                          sys_time_t time_out) {
  struct proc *proc = sched_cur_proc();
  struct buffer *buf = buffer_from_id(buffer_id);
  if (buf == NULL || buf->buf_part_id != sched_cur_part()->pa_id) {
    return INVALID_PARAM;
  }
  if (length > buf->buf_max_msg_size) {
    return INVALID_PARAM;
  }
  if (length == 0) {
    return INVALID_PARAM;
  }
  panic_e(lk_acquire(&buf->buf_lock));
  if (buffer_is_full(buf)) {
    if (time_out == 0) {
      panic_e(lk_release(&buf->buf_lock));
      return NOT_AVAILABLE;
      // TODO: check if proc owns a mutex or is error handler
    } else {
      proc->pr_state = WAITING;
      buffer_add_waiting_proc(buf, proc);
      sys_time_t wakeup_time = boottime_get_now() + time_out;
      if (time_out != SYSTEM_TIME_INFINITE_VAL) {
        proc->pr_waiting_reason = BUFFER_BLOCKED_WITH_TIMEOUT;
        struct te_proc_buf tepb = {.tepb_proc = proc, .tepb_buf = buf};
        time_event_alloc(&tepb, wakeup_time, TE_BUFFER_BLOCK_TIMEOUT);
      } else {
        proc->pr_waiting_reason = BUFFER_BLOCKED;
      }
      panic_e(lk_release(&buf->buf_lock));
      sched_proc_give_up(0);
      if (time_out != SYSTEM_TIME_INFINITE_VAL && boottime_get_now() > wakeup_time) {
        return TIMED_OUT;
      }
    }
  } else {
    panic_e(lk_release(&buf->buf_lock));
  }
  buffer_send(buf, message_addr, length);
  panic_e(lk_acquire(&buf->buf_lock));
  struct proc *to_wakeup = buffer_wakeup_waiting_proc(buf);
  if (to_wakeup != NULL) {
    if (to_wakeup->pr_waiting_reason == BUFFER_BLOCKED_WITH_TIMEOUT) {
      time_event_free(to_wakeup->pr_asso_timer);
    }
    to_wakeup->pr_state = READY;
    panic_e(lk_release(&buf->buf_lock));
    sched_proc_give_up(0);
  } else {
    panic_e(lk_release(&buf->buf_lock));
  }
  return NO_ERROR;
}

ret_code_t do_receive_buffer(buf_id_t buffer_id, sys_time_t time_out, msg_addr_t message_addr,
                             msg_size_t *length) {
  struct proc *proc = sched_cur_proc();
  struct buffer *buf = buffer_from_id(buffer_id);
  if (buf == NULL || buf->buf_part_id != sched_cur_part()->pa_id) {
    return INVALID_PARAM;
  }
  panic_e(lk_acquire(&buf->buf_lock));
  if (buffer_is_empty(buf)) {
    if (time_out == 0) {
      panic_e(lk_release(&buf->buf_lock));
      *length = 0;
      return NOT_AVAILABLE;
      // TODO: check if proc owns a mutex or is error handler
    } else {
      proc->pr_state = WAITING;
      buffer_add_waiting_proc(buf, proc);
      sys_time_t wakeup_time = boottime_get_now() + time_out;
      if (time_out != SYSTEM_TIME_INFINITE_VAL) {
        proc->pr_waiting_reason = BUFFER_BLOCKED_WITH_TIMEOUT;
        struct te_proc_buf tepb = {.tepb_proc = proc, .tepb_buf = buf};
        time_event_alloc(&tepb, wakeup_time, TE_BUFFER_BLOCK_TIMEOUT);
      } else {
        proc->pr_waiting_reason = BUFFER_BLOCKED;
      }
      panic_e(lk_release(&buf->buf_lock));
      sched_proc_give_up(0);
      if (time_out != SYSTEM_TIME_INFINITE_VAL && boottime_get_now() > wakeup_time) {
        *length = 0;
        return TIMED_OUT;
      }
    }
  } else {
    panic_e(lk_release(&buf->buf_lock));
  }
  buffer_recv(buf, message_addr, length);
  panic_e(lk_acquire(&buf->buf_lock));
  struct proc *to_wakeup = buffer_wakeup_waiting_proc(buf);
  if (to_wakeup != NULL) {
    if (to_wakeup->pr_waiting_reason == BUFFER_BLOCKED_WITH_TIMEOUT) {
      time_event_free(to_wakeup->pr_asso_timer);
    }
    to_wakeup->pr_state = READY;
    panic_e(lk_release(&buf->buf_lock));
    sched_proc_give_up(0);
  } else {
    panic_e(lk_release(&buf->buf_lock));
  }
  return NO_ERROR;
}

ret_code_t do_get_buffer_id(buf_name_t buffer_name, buf_id_t *buffer_id) {
  struct buffer *buf = part_get_buf_by_name(sched_cur_part(), buffer_name);
  if (buf == NULL) {
    return INVALID_CONFIG;
  }
  *buffer_id = buf->buf_id;
  return NO_ERROR;
}

ret_code_t do_get_buffer_status(buf_id_t buffer_id, BUFFER_STATUS_TYPE *buffer_status) {
  struct buffer *buf = buffer_from_id(buffer_id);
  if (buf == NULL || buf->buf_part_id != sched_cur_part()->pa_id) {
    return INVALID_PARAM;
  }
  buffer_status->NB_MESSAGE = buf->buf_nb_msg;
  buffer_status->MAX_NB_MESSAGE = buf->buf_max_nb_msg;
  buffer_status->MAX_MESSAGE_SIZE = buf->buf_max_msg_size;
  buffer_status->WAITING_PROCESSES = buffer_get_waiting_proc_nb(buf);
  return NO_ERROR;
}

ret_code_t do_create_blackboard(bb_name_t blackboard_name, msg_size_t max_message_size,
                                bb_id_t *blackboard_id) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_display_blackboard(bb_id_t blackboard_id, msg_addr_t message_addr,
                                 msg_size_t length) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_read_blackboard(bb_id_t blackboard_id, sys_time_t time_out,
                              msg_addr_t message_addr, msg_size_t *length) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_clear_blackboard(bb_id_t blackboard_id) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_get_blackboard_id(bb_name_t blackboard_name, bb_id_t *blackboard_id) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_get_blackboard_status(bb_id_t blackboard_id,
                                    BLACKBOARD_STATUS_TYPE *blackboard_status) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_create_semaphore(sem_name_t semaphore_name, sem_value_t current_value,
                               sem_value_t maximum_value, que_disc_t queuing_discipline,
                               sem_id_t *semaphore_id) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_wait_semaphore(sem_id_t semaphore_id, sys_time_t time_out) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_signal_semaphore(sem_id_t semaphore_id) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_get_semaphore_id(sem_name_t semaphore_name, sem_id_t *semaphore_id) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_get_semaphore_status(sem_id_t semaphore_id,
                                   SEMAPHORE_STATUS_TYPE *semaphore_status) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_create_event(event_name_t event_name, event_id_t *event_id) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_set_event(event_id_t event_id) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_reset_event(event_id_t event_id) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_wait_event(event_id_t event_id, sys_time_t time_out) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_get_event_id(event_name_t event_name, event_id_t *event_id) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_get_event_status(event_id_t event_id, EVENT_STATUS_TYPE *event_status) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_create_mutex(mutex_name_t mutex_name, priority_t mutex_priority,
                           que_disc_t queuing_discipline, mutex_id_t *mutex_id) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_acquire_mutex(mutex_id_t mutex_id, sys_time_t time_out) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_release_mutex(mutex_id_t mutex_id) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_reset_mutex(mutex_id_t mutex_id, proc_id_t process_id) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_get_mutex_id(mutex_name_t mutex_name, mutex_id_t *mutex_id) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_get_mutex_status(mutex_id_t mutex_id, MUTEX_STATUS_TYPE *mutex_status) {
  // TODO
  return NO_ERROR;
}

ret_code_t do_get_process_mutex_state(proc_id_t process_id, mutex_id_t *mutex_id) {
  // TODO
  return NO_ERROR;
}

void do_syscall(struct context *context) {
  // TODO: check pointer and enum from user space
  context->ctx_sepc += sizeof(uint32_t);
  unsigned long sysno = context->ctx_regs.names.a7;
  ret_code_t ret = NO_ERROR;
  switch (sysno) {
  case SYS_CONS_WRITE_CHAR:
    do_cons_write_char(context->ctx_regs.names.a0);
    break;
  case SYS_CONS_READ_CHAR:
    ret = do_cons_read_char();
    break;
  case SYS_CONS_WRITE_BUF:
    do_cons_write_buf((char *)context->ctx_regs.names.a0, context->ctx_regs.names.a1);
    break;
  case SYS_HALT:
    do_halt();
    break;
  case SYS_GET_PARTITION_STATUS:
    ret = do_get_partition_status((PARTITION_STATUS_TYPE *)context->ctx_regs.names.a0);
    break;
  case SYS_SET_PARTITION_MODE:
    ret = do_set_partition_mode(context->ctx_regs.names.a0);
    break;
  case SYS_GET_PROCESS_ID:
    ret = do_get_process_id((proc_name_t)context->ctx_regs.names.a0,
                            (proc_id_t *)context->ctx_regs.names.a1);
    break;
  case SYS_GET_PROCESS_STATUS:
    ret = do_get_process_status(context->ctx_regs.names.a0,
                                (PROCESS_STATUS_TYPE *)context->ctx_regs.names.a1);
    break;
  case SYS_CREATE_PROCESS:
    ret = do_create_process((PROCESS_ATTRIBUTE_TYPE *)context->ctx_regs.names.a0,
                            (proc_id_t *)context->ctx_regs.names.a1);
    break;
  case SYS_SET_PRIORITY:
    ret = do_set_priority(context->ctx_regs.names.a0, context->ctx_regs.names.a1);
    break;
  case SYS_SUSPEND_SELF:
    ret = do_suspend_self(context->ctx_regs.names.a0);
    break;
  case SYS_SUSPEND:
    ret = do_suspend(context->ctx_regs.names.a0);
    break;
  case SYS_RESUME:
    ret = do_resume(context->ctx_regs.names.a0);
    break;
  case SYS_STOP_SELF:
    do_stop_self();
    break;
  case SYS_STOP:
    ret = do_stop(context->ctx_regs.names.a0);
    break;
  case SYS_START:
    ret = do_start(context->ctx_regs.names.a0);
    break;
  case SYS_DELAYED_START:
    ret = do_delayed_start(context->ctx_regs.names.a0, context->ctx_regs.names.a1);
    break;
  case SYS_LOCK_PREEMPTION:
    ret = do_lock_preemption((lock_level_t *)context->ctx_regs.names.a0);
    break;
  case SYS_UNLOCK_PREEMPTION:
    ret = do_unlock_preemption((lock_level_t *)context->ctx_regs.names.a0);
    break;
  case SYS_GET_MY_ID:
    ret = do_get_my_id((proc_id_t *)context->ctx_regs.names.a0);
    break;
  case SYS_INITIALIZE_PROCESS_CORE_AFFINITY:
    ret = do_initialize_process_core_affinity(context->ctx_regs.names.a0,
                                              context->ctx_regs.names.a1);
    break;
  case SYS_GET_MY_PROCESSOR_CORE_ID:
    ret = do_get_my_processor_core_id((proc_core_id_t *)context->ctx_regs.names.a0);
    break;
  case SYS_GET_MY_INDEX:
    ret = do_get_my_index((proc_index_t *)context->ctx_regs.names.a0);
    break;
  case SYS_TIMED_WAIT:
    ret = do_timed_wait(context->ctx_regs.names.a0);
    break;
  case SYS_PERIODIC_WAIT:
    ret = do_periodic_wait();
    break;
  case SYS_GET_TIME:
    ret = do_get_time((sys_time_t *)context->ctx_regs.names.a0);
    break;
  case SYS_REPLENISH:
    ret = do_replenish(context->ctx_regs.names.a0);
    break;
  case SYS_CREATE_SAMPLING_PORT:
    ret = do_create_sampling_port((samp_port_name_t)context->ctx_regs.names.a0,
                                  context->ctx_regs.names.a1, context->ctx_regs.names.a2,
                                  context->ctx_regs.names.a3,
                                  (samp_port_id_t *)context->ctx_regs.names.a4);
    break;
  case SYS_WRITE_SAMPLING_MESSAGE:
    ret = do_write_sampling_message(context->ctx_regs.names.a0,
                                    (msg_addr_t)context->ctx_regs.names.a1,
                                    context->ctx_regs.names.a2);
    break;
  case SYS_READ_SAMPLING_MESSAGE:
    ret = do_read_sampling_message(
        context->ctx_regs.names.a0, (msg_addr_t)context->ctx_regs.names.a1,
        (msg_size_t *)context->ctx_regs.names.a2, (validity_t *)context->ctx_regs.names.a3);
    break;
  case SYS_GET_SAMPLING_PORT_ID:
    ret = do_get_sampling_port_id((samp_port_name_t)context->ctx_regs.names.a0,
                                  (samp_port_id_t *)context->ctx_regs.names.a1);
    break;
  case SYS_GET_SAMPLING_PORT_STATUS:
    ret = do_get_sampling_port_status(context->ctx_regs.names.a0,
                                      (SAMPLING_PORT_STATUS_TYPE *)context->ctx_regs.names.a1);
    break;
  case SYS_CREATE_QUEUING_PORT:
    ret = do_create_queuing_port((que_port_name_t)context->ctx_regs.names.a0,
                                 context->ctx_regs.names.a1, context->ctx_regs.names.a2,
                                 context->ctx_regs.names.a3, context->ctx_regs.names.a4,
                                 (que_port_id_t *)context->ctx_regs.names.a5);
    break;
  case SYS_SEND_QUEUING_MESSAGE:
    ret = do_send_queuing_message(context->ctx_regs.names.a0,
                                  (msg_addr_t)context->ctx_regs.names.a1,
                                  context->ctx_regs.names.a2, context->ctx_regs.names.a3);
    break;
  case SYS_RECEIVE_QUEUING_MESSAGE:
    ret = do_receive_queuing_message(context->ctx_regs.names.a0, context->ctx_regs.names.a1,
                                     (msg_addr_t)context->ctx_regs.names.a2,
                                     (msg_size_t *)context->ctx_regs.names.a3);
    break;
  case SYS_GET_QUEUING_PORT_ID:
    ret = do_get_queuing_port_id((que_port_name_t)context->ctx_regs.names.a0,
                                 (que_port_id_t *)context->ctx_regs.names.a1);
    break;
  case SYS_GET_QUEUING_PORT_STATUS:
    ret = do_get_queuing_port_status(context->ctx_regs.names.a0,
                                     (QUEUING_PORT_STATUS_TYPE *)context->ctx_regs.names.a1);
    break;
  case SYS_CLEAR_QUEUING_PORT:
    ret = do_clear_queuing_port(context->ctx_regs.names.a0);
    break;
  case SYS_CREATE_BUFFER:
    ret = do_create_buffer((buf_name_t)context->ctx_regs.names.a0, context->ctx_regs.names.a1,
                           context->ctx_regs.names.a2, context->ctx_regs.names.a3,
                           (buf_id_t *)context->ctx_regs.names.a4);
    break;
  case SYS_SEND_BUFFER:
    ret = do_send_buffer(context->ctx_regs.names.a0, (msg_addr_t)context->ctx_regs.names.a1,
                         context->ctx_regs.names.a2, context->ctx_regs.names.a3);
    break;
  case SYS_RECEIVE_BUFFER:
    ret = do_receive_buffer(context->ctx_regs.names.a0, context->ctx_regs.names.a1,
                            (msg_addr_t)context->ctx_regs.names.a2,
                            (msg_size_t *)context->ctx_regs.names.a3);
    break;
  case SYS_GET_BUFFER_ID:
    ret = do_get_buffer_id((buf_name_t)context->ctx_regs.names.a0,
                           (buf_id_t *)context->ctx_regs.names.a1);
    break;
  case SYS_GET_BUFFER_STATUS:
    ret = do_get_buffer_status(context->ctx_regs.names.a0,
                               (BUFFER_STATUS_TYPE *)context->ctx_regs.names.a1);
    break;
  case SYS_CREATE_BLACKBOARD:
    ret =
        do_create_blackboard((bb_name_t)context->ctx_regs.names.a0, context->ctx_regs.names.a1,
                             (bb_id_t *)context->ctx_regs.names.a2);
    break;
  case SYS_DISPLAY_BLACKBOARD:
    ret = do_display_blackboard(context->ctx_regs.names.a0,
                                (msg_addr_t)context->ctx_regs.names.a1,
                                context->ctx_regs.names.a2);
    break;
  case SYS_READ_BLACKBOARD:
    ret = do_read_blackboard(context->ctx_regs.names.a0, context->ctx_regs.names.a1,
                             (msg_addr_t)context->ctx_regs.names.a2,
                             (msg_size_t *)context->ctx_regs.names.a3);
    break;
  case SYS_CLEAR_BLACKBOARD:
    ret = do_clear_blackboard(context->ctx_regs.names.a0);
    break;
  case SYS_GET_BLACKBOARD_ID:
    ret = do_get_blackboard_id((bb_name_t)context->ctx_regs.names.a0,
                               (bb_id_t *)context->ctx_regs.names.a1);
    break;
  case SYS_GET_BLACKBOARD_STATUS:
    ret = do_get_blackboard_status(context->ctx_regs.names.a0,
                                   (BLACKBOARD_STATUS_TYPE *)context->ctx_regs.names.a1);
    break;
  case SYS_CREATE_SEMAPHORE:
    ret =
        do_create_semaphore((sem_name_t)context->ctx_regs.names.a0, context->ctx_regs.names.a1,
                            context->ctx_regs.names.a2, context->ctx_regs.names.a3,
                            (sem_id_t *)context->ctx_regs.names.a4);
    break;
  case SYS_WAIT_SEMAPHORE:
    ret = do_wait_semaphore(context->ctx_regs.names.a0, context->ctx_regs.names.a1);
    break;
  case SYS_SIGNAL_SEMAPHORE:
    ret = do_signal_semaphore(context->ctx_regs.names.a0);
    break;
  case SYS_GET_SEMAPHORE_ID:
    ret = do_get_semaphore_id((sem_name_t)context->ctx_regs.names.a0,
                              (sem_id_t *)context->ctx_regs.names.a1);
    break;
  case SYS_GET_SEMAPHORE_STATUS:
    ret = do_get_semaphore_status(context->ctx_regs.names.a0,
                                  (SEMAPHORE_STATUS_TYPE *)context->ctx_regs.names.a1);
    break;
  case SYS_CREATE_EVENT:
    ret = do_create_event((event_name_t)context->ctx_regs.names.a0,
                          (event_id_t *)context->ctx_regs.names.a1);
    break;
  case SYS_SET_EVENT:
    ret = do_set_event(context->ctx_regs.names.a0);
    break;
  case SYS_RESET_EVENT:
    ret = do_reset_event(context->ctx_regs.names.a0);
    break;
  case SYS_WAIT_EVENT:
    ret = do_wait_event(context->ctx_regs.names.a0, context->ctx_regs.names.a1);
    break;
  case SYS_GET_EVENT_ID:
    ret = do_get_event_id((event_name_t)context->ctx_regs.names.a0,
                          (event_id_t *)context->ctx_regs.names.a1);
    break;
  case SYS_GET_EVENT_STATUS:
    ret = do_get_event_status(context->ctx_regs.names.a0,
                              (EVENT_STATUS_TYPE *)context->ctx_regs.names.a1);
    break;
  case SYS_CREATE_MUTEX:
    ret = do_create_mutex((mutex_name_t)context->ctx_regs.names.a0, context->ctx_regs.names.a1,
                          context->ctx_regs.names.a2, (mutex_id_t *)context->ctx_regs.names.a3);
    break;
  case SYS_ACQUIRE_MUTEX:
    ret = do_acquire_mutex(context->ctx_regs.names.a0, context->ctx_regs.names.a1);
    break;
  case SYS_RELEASE_MUTEX:
    ret = do_release_mutex(context->ctx_regs.names.a0);
    break;
  case SYS_RESET_MUTEX:
    ret = do_reset_mutex(context->ctx_regs.names.a0, context->ctx_regs.names.a1);
    break;
  case SYS_GET_MUTEX_ID:
    ret = do_get_mutex_id((mutex_name_t)context->ctx_regs.names.a0,
                          (mutex_id_t *)context->ctx_regs.names.a1);
    break;
  case SYS_GET_MUTEX_STATUS:
    ret = do_get_mutex_status(context->ctx_regs.names.a0,
                              (MUTEX_STATUS_TYPE *)context->ctx_regs.names.a1);
    break;
  case SYS_GET_PROCESS_MUTEX_STATE:
    ret = do_get_process_mutex_state(context->ctx_regs.names.a0,
                                     (mutex_id_t *)context->ctx_regs.names.a1);
    break;
  default:
    ret = INVALID_SYSNO;
    break;
  }
  context->ctx_regs.names.a0 = ret;
}

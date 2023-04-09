#include <kern/drivers/serialport.h>
#include <kern/lib/boottime.h>
#include <kern/lib/debug.h>
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
    sched_proc_give_up();
    break;
  default:
    return INVALID_PARAM;
    break;
  }
  return NO_ERROR;
}

ret_code_t do_get_process_id(proc_name_t process_name, proc_id_t *process_id) {
  struct part *part = sched_cur_part();
  struct linked_node *proc_node = hashmap_get(&part->pa_proc_name_map, process_name);
  if (proc_node == NULL) {
    return INVALID_CONFIG;
  }
  struct proc *proc = CONTAINER_OF(proc_node, struct proc, pr_name_link);
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
  strcpy(process_status->ATTRIBUTES.NAME, proc->pr_name);
  return NO_ERROR;
}

ret_code_t do_create_process(PROCESS_ATTRIBUTE_TYPE *attributes, proc_id_t *process_id) {
  struct part *part = sched_cur_part();
  struct linked_node *proc_node = hashmap_get(&part->pa_proc_name_map, attributes->NAME);
  if (proc_node != NULL) {
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
  struct proc *proc;
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
  sched_proc_give_up();
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
    sched_proc_give_up();
    if (boottime_get_now() >= wakeup_time) {
      return TIMED_OUT;
    }
  } else {
    sched_proc_give_up();
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
  sched_proc_give_up();
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
    time_event_proc_free_filter_type(proc, TE_PROCESS_SUSPEND_TIMEOUT);
  }
  // TODO: check if proc is waiting on a process queue or TIMED_WAIT time delay or DELAYED_START
  // time delay
  proc->pr_state = READY;
  sched_proc_give_up();
  return NO_ERROR;
}

void do_stop_self(void) {
  struct proc *proc = sched_cur_proc();
  // TODO: check if proc owns preemption lock mutex
  proc->pr_state = DORMANT;
  // TODO: prevent proc from causing a deadline overrun fault
  sched_proc_give_up();
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
  time_event_proc_free_filter_type(proc, TE_ANY);
  // TODO: prevent the process from causing any deadline overrun faults
  sched_proc_give_up();
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
      sched_proc_give_up();
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
  // TODO
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
  default:
    ret = INVALID_SYSNO;
    break;
  }
  context->ctx_regs.names.a0 = ret;
}

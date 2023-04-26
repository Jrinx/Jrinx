#ifndef _KERNEL_COMM_BLACKBOARD_H_
#define _KERNEL_COMM_BLACKBOARD_H_

#include <kern/lock/lock.h>
#include <list.h>
#include <types.h>

struct blackboard {
  bb_name_t bb_name;
  bb_id_t bb_id;
  part_id_t bb_part_id;
  empty_ind_t bb_empty_ind;
  msg_size_t bb_max_msg_size;
  void *bb_data;
  size_t bb_cap;
  size_t bb_len;
  struct lock bb_lock;
  struct list_head bb_waiting_procs;
  struct linked_node bb_id_link;
  struct linked_node bb_name_link;
};

struct part;
struct proc;

struct blackboard *blackboard_from_id(bb_id_t bb_id);
long blackboard_alloc(struct part *part, struct blackboard **bb, bb_name_t name,
                      msg_size_t max_msg_size) __attribute__((warn_unused_result));
long blackboard_free(struct blackboard *bb) __attribute__((warn_unused_result));
int blackboard_is_full(struct blackboard *bb);
int blackboard_is_empty(struct blackboard *bb);
void blackboard_display(struct blackboard *bb, msg_addr_t msg_addr, msg_size_t msg_len);
void blackboard_read(struct blackboard *bb, msg_addr_t msg_addr, msg_size_t *msg_len);
void blackboard_clear(struct blackboard *bb);
void blackboard_add_waiting_proc(struct blackboard *bb, struct proc *proc);
void blackboard_del_waiting_proc(struct blackboard *bb, struct proc *proc);
struct proc *blackboard_wakeup_waiting_proc(struct blackboard *bb);
wait_range_t blackboard_get_waiting_proc_nb(struct blackboard *bb);

#endif

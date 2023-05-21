#ifndef _KERN_COMM_BUFFER_H_
#define _KERN_COMM_BUFFER_H_

#include <attr.h>
#include <kern/lock/lock.h>
#include <layouts.h>
#include <lib/circbuf.h>
#include <list.h>
#include <types.h>

struct buffer {
  buf_name_t buf_name;
  buf_id_t buf_id;
  part_id_t buf_part_id;
  que_disc_t buf_que_disc;
  msg_size_t buf_max_msg_size;
  msg_range_t buf_max_nb_msg;
  struct circbuf buf_body;
  struct lock buf_lock;
  struct list_head buf_waiting_procs;
  struct linked_node buf_id_link;
  struct linked_node buf_name_link;
};

struct part;
struct proc;

struct buffer *buffer_from_id(buf_id_t buf_id);
long buffer_alloc(struct part *part, struct buffer **buf, buf_name_t name,
                  msg_size_t max_msg_size, msg_range_t max_nb_msg,
                  que_disc_t que_disc) __warn_unused_result;
long buffer_free(struct buffer *buf) __warn_unused_result;
int buffer_is_full(struct buffer *buf);
int buffer_is_empty(struct buffer *buf);
void buffer_send(struct buffer *buf, msg_addr_t msg_addr, msg_size_t msg_len);
void buffer_recv(struct buffer *buf, msg_addr_t msg_addr, msg_size_t *msg_len);
void buffer_add_waiting_proc(struct buffer *buf, struct proc *proc);
void buffer_del_waiting_proc(struct buffer *buf, struct proc *proc);
struct proc *buffer_wakeup_waiting_proc(struct buffer *buf);
wait_range_t buffer_get_waiting_proc_nb(struct buffer *buf);
msg_range_t buffer_get_nb_message(struct buffer *buf);

#endif

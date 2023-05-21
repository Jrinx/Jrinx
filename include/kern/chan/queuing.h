#ifndef _KERN_CHAN_QUEUING_H_
#define _KERN_CHAN_QUEUING_H_

#include <attr.h>
#include <kern/chan/channel.h>
#include <kern/multitask/partition.h>
#include <types.h>

struct queuing_port {
  que_port_id_t qp_id;
  que_port_name_t qp_name;
  part_id_t qp_part_id;
  port_dir_t qp_dir;
  que_disc_t qp_disc;
  struct channel *qp_channel;
  struct linked_node qp_id_link;
  struct linked_node qp_name_link;
};

struct queuing_port_conf {
  char *qpc_name;
  char *qpc_pa_name;
  port_dir_t qpc_dir;
  msg_size_t qpc_max_msg_size;
  msg_range_t qpc_max_nb_msg;
  struct channel *qpc_channel;
  struct linked_node qpc_link;
};

struct queuing_port *queuing_port_from_id(que_port_id_t qp_id);
struct queuing_port *queuing_port_from_name(const char *qp_name);

int queuing_port_conf_validate(struct part *part, que_port_name_t port_name,
                               port_dir_t direction, msg_size_t max_msg_size,
                               msg_range_t max_nb_msg);
long queuing_port_alloc(struct part *part, struct queuing_port **qpp, que_port_name_t port_name,
                        port_dir_t direction, msg_size_t max_msg_size, msg_range_t max_nb_msg,
                        que_disc_t disc) __warn_unused_result;
long queuing_port_conf_chan(que_port_name_t port_name,
                            struct channel *chan) __warn_unused_result;
void queuing_port_register(struct queuing_port_conf *qpc);
int queuing_port_is_full(struct queuing_port *qp);
int queuing_port_is_empty(struct queuing_port *qp);
void queuing_port_send(struct queuing_port *qp, msg_addr_t msg_addr, msg_size_t msg_len);
void queuing_port_recv(struct queuing_port *qp, msg_addr_t msg_addr, msg_size_t *msg_len);
void queuing_port_add_waiting_proc(struct queuing_port *qp, struct proc *proc);
void queuing_port_del_waiting_proc(struct queuing_port *qp, struct proc *proc);
struct proc *queuing_port_wakeup_waiting_proc(struct queuing_port *qp);
wait_range_t queuing_port_get_wait_proc_nb(struct queuing_port *qp);
msg_range_t queuing_port_get_nb_message(struct queuing_port *qp);

#endif

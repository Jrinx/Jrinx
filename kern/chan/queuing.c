#include <kern/chan/queuing.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lock/lock.h>
#include <kern/lock/spinlock.h>
#include <kern/mm/kalloc.h>
#include <kern/multitask/process.h>
#include <lib/hashmap.h>
#include <lib/string.h>
#include <list.h>

static const void *queuing_port_id_key_of(const struct linked_node *node) {
  const struct queuing_port *qp = CONTAINER_OF(node, struct queuing_port, qp_id_link);
  return &qp->qp_id;
}

static const void *queuing_port_name_key_of(const struct linked_node *node) {
  const struct queuing_port *qp = CONTAINER_OF(node, struct queuing_port, qp_name_link);
  return qp->qp_name;
}

static struct hlist_head queuing_port_id_map_array[32];
static struct hashmap queuing_port_id_map = {
    .h_array = queuing_port_id_map_array,
    .h_num = 0,
    .h_cap = 32,
    .h_code = hash_code_uint64,
    .h_equals = hash_eq_uint64,
    .h_key = queuing_port_id_key_of,
};

static struct hlist_head queuing_port_name_map_array[32];
static struct hashmap queuing_port_name_map = {
    .h_array = queuing_port_name_map_array,
    .h_num = 0,
    .h_cap = 32,
    .h_code = hash_code_str,
    .h_equals = hash_eq_str,
    .h_key = queuing_port_name_key_of,
};

static struct hlist_head queuing_port_conf_list;

static uint64_t queuing_port_id_alloc(void) {
  static uint64_t nxt_id = 1;
  static with_spinlock(nxt_id);
  uint64_t ret;
  panic_e(lk_acquire(&spinlock_of(nxt_id)));
  ret = nxt_id;
  nxt_id++;
  panic_e(lk_release(&spinlock_of(nxt_id)));
  return ret;
}

struct queuing_port *queuing_port_from_id(que_port_id_t qp_id) {
  struct linked_node *node = hashmap_get(&queuing_port_id_map, &qp_id);
  if (node == NULL) {
    return NULL;
  }
  struct queuing_port *qp = CONTAINER_OF(node, struct queuing_port, qp_id_link);
  return qp;
}

struct queuing_port *queuing_port_from_name(const char *name) {
  struct linked_node *node = hashmap_get(&queuing_port_name_map, name);
  if (node == NULL) {
    return NULL;
  }
  struct queuing_port *qp = CONTAINER_OF(node, struct queuing_port, qp_name_link);
  return qp;
}

int queuing_port_conf_validate(struct part *part, que_port_name_t port_name,
                               port_dir_t direction, msg_size_t max_msg_size,
                               msg_range_t max_nb_msg) {
  struct queuing_port_conf *p;
  LINKED_NODE_ITER (queuing_port_conf_list.h_first, p, qpc_link) {
    if (p->qpc_dir != direction) {
      continue;
    }
    if (p->qpc_max_msg_size != max_msg_size) {
      continue;
    }
    if (p->qpc_max_nb_msg != max_nb_msg) {
      continue;
    }
    if (strcmp(p->qpc_pa_name, part->pa_name) != 0) {
      continue;
    }
    if (strcmp(p->qpc_name, port_name) != 0) {
      continue;
    }
    return 1;
  }
  return 0;
}

long queuing_port_alloc(struct part *part, struct queuing_port **qpp, que_port_name_t port_name,
                        port_dir_t direction, msg_size_t max_msg_size, msg_range_t max_nb_msg,
                        que_disc_t disc) {
  struct channel *chan = NULL;
  struct queuing_port_conf *qpc;
  LINKED_NODE_ITER (queuing_port_conf_list.h_first, qpc, qpc_link) {
    if (strcmp(qpc->qpc_name, port_name) == 0) {
      chan = qpc->qpc_channel;
      break;
    }
  }
  if (chan == NULL) {
    return -KER_PORT_ER;
  }
  struct queuing_port *tmp = kalloc(sizeof(struct queuing_port));
  memset(tmp, 0, sizeof(struct queuing_port));
  tmp->qp_id = queuing_port_id_alloc();
  tmp->qp_name = kalloc((strlen(port_name) + 1) * sizeof(char));
  strcpy(tmp->qp_name, port_name);
  tmp->qp_part_id = part->pa_id;
  tmp->qp_dir = direction;
  tmp->qp_disc = disc;
  tmp->qp_channel = chan;
  hashmap_put(&queuing_port_id_map, &tmp->qp_id_link);
  hashmap_put(&queuing_port_name_map, &tmp->qp_name_link);
  *qpp = tmp;
  return KER_SUCCESS;
}

long queuing_port_conf_chan(que_port_name_t port_name, struct channel *chan) {
  struct queuing_port_conf *qpc;
  LINKED_NODE_ITER (queuing_port_conf_list.h_first, qpc, qpc_link) {
    if (strcmp(qpc->qpc_name, port_name) == 0) {
      qpc->qpc_channel = chan;
      chan->ch_view.queuing.ch_max_msg_size = qpc->qpc_max_msg_size;
      chan->ch_view.queuing.ch_max_nb_msg = qpc->qpc_max_nb_msg;
      return KER_SUCCESS;
    }
  }
  return -KER_PORT_ER;
}

void queuing_port_register(struct queuing_port_conf *qpc) {
  info("register queuing port: "
       "name='%s',partition='%s',direction=%s,max-msg-size=%pB,max-nb-msg=%lu\n",
       qpc->qpc_name, qpc->qpc_pa_name, qpc->qpc_dir == SOURCE ? "SRC" : "DST",
       &qpc->qpc_max_msg_size, qpc->qpc_max_nb_msg);
  struct queuing_port_conf *tmp = kalloc(sizeof(struct queuing_port_conf));
  memset(tmp, 0, sizeof(struct queuing_port_conf));
  tmp->qpc_name = kalloc((strlen(qpc->qpc_name) + 1) * sizeof(char));
  strcpy(tmp->qpc_name, qpc->qpc_name);
  tmp->qpc_pa_name = kalloc((strlen(qpc->qpc_pa_name) + 1) * sizeof(char));
  strcpy(tmp->qpc_pa_name, qpc->qpc_pa_name);
  tmp->qpc_dir = qpc->qpc_dir;
  tmp->qpc_max_msg_size = qpc->qpc_max_msg_size;
  tmp->qpc_max_nb_msg = qpc->qpc_max_nb_msg;
  hlist_insert_head(&queuing_port_conf_list, &tmp->qpc_link);
}

int queuing_port_is_full(struct queuing_port *qp) {
  return circbuf_is_full(&qp->qp_channel->ch_view.queuing.ch_body);
}

int queuing_port_is_empty(struct queuing_port *qp) {
  return circbuf_is_empty(&qp->qp_channel->ch_view.queuing.ch_body);
}

void queuing_port_send(struct queuing_port *qp, msg_addr_t msg_addr, msg_size_t msg_len) {
  assert(!queuing_port_is_full(qp));
  assert(qp->qp_dir == SOURCE);
  uintptr_t tail = circbuf_enqu_st(&qp->qp_channel->ch_view.queuing.ch_body);
  struct comm_msg *msg =
      (struct comm_msg *)(qp->qp_channel->ch_view.queuing.ch_body.cc_buf + tail);
  msg->msg_size = msg_len;
  memcpy(msg->msg_data, msg_addr, msg_len);
  circbuf_enqu_ed(&qp->qp_channel->ch_view.queuing.ch_body);
}

void queuing_port_recv(struct queuing_port *qp, msg_addr_t msg_addr, msg_size_t *msg_len) {
  assert(!queuing_port_is_empty(qp));
  assert(qp->qp_dir == DESTINATION);
  uintptr_t head = circbuf_dequ_st(&qp->qp_channel->ch_view.queuing.ch_body);
  struct comm_msg *msg =
      (struct comm_msg *)(qp->qp_channel->ch_view.queuing.ch_body.cc_buf + head);
  *msg_len = msg->msg_size;
  memcpy(msg_addr, msg->msg_data, *msg_len);
  circbuf_dequ_ed(&qp->qp_channel->ch_view.queuing.ch_body);
}

void queuing_port_add_waiting_proc(struct queuing_port *qp, struct proc *proc) {
  list_insert_tail(&qp->qp_channel->ch_view.queuing.ch_waiting_procs, &proc->pr_wait_chan_link);
}

void queuing_port_del_waiting_proc(struct queuing_port *qp, struct proc *proc) {
  list_remove_node(&qp->qp_channel->ch_view.queuing.ch_waiting_procs, &proc->pr_wait_chan_link);
}

struct proc *queuing_port_wakeup_waiting_proc(struct queuing_port *qp) {
  if (list_empty(&qp->qp_channel->ch_view.queuing.ch_waiting_procs)) {
    return NULL;
  }
  struct proc *proc;
  if (qp->qp_disc == FIFO) {
    struct linked_node *node = qp->qp_channel->ch_view.queuing.ch_waiting_procs.l_first;
    proc = CONTAINER_OF(node, struct proc, pr_wait_chan_link);
    list_remove_node(&qp->qp_channel->ch_view.queuing.ch_waiting_procs, node);
    return proc;
  } else {
    struct proc *max_pri_proc = NULL;
    LINKED_NODE_ITER (qp->qp_channel->ch_view.queuing.ch_waiting_procs.l_first, proc,
                      pr_wait_chan_link) {
      if (max_pri_proc == NULL || proc->pr_cur_pri > max_pri_proc->pr_cur_pri) {
        max_pri_proc = proc;
      }
    }
    list_remove_node(&qp->qp_channel->ch_view.queuing.ch_waiting_procs,
                     &max_pri_proc->pr_wait_chan_link);
    return max_pri_proc;
  }
}

wait_range_t queuing_port_get_wait_proc_nb(struct queuing_port *qp) {
  wait_range_t nb = 0;
  struct proc *proc;
  LINKED_NODE_ITER (qp->qp_channel->ch_view.queuing.ch_waiting_procs.l_first, proc,
                    pr_wait_chan_link) {
    nb++;
  }
  return nb;
}

msg_range_t queuing_port_get_nb_message(struct queuing_port *qp) {
  return qp->qp_channel->ch_view.queuing.ch_body.cc_cnt;
}

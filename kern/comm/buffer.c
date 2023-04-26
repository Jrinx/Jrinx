#include <kern/comm/buffer.h>
#include <kern/lib/debug.h>
#include <kern/lock/lock.h>
#include <kern/lock/spinlock.h>
#include <kern/mm/kalloc.h>
#include <kern/mm/pmm.h>
#include <kern/mm/vmm.h>
#include <kern/multitask/partition.h>
#include <kern/multitask/process.h>
#include <layouts.h>
#include <lib/hashmap.h>
#include <lib/string.h>
#include <magicros.h>

static const void *buf_id_key_of(const struct linked_node *node) {
  const struct buffer *buf = CONTAINER_OF(node, struct buffer, buf_id_link);
  return &buf->buf_id;
}

static struct hlist_head buf_id_map_array[64];
static struct hashmap buf_id_map = {
    .h_array = buf_id_map_array,
    .h_num = 0,
    .h_cap = 64,
    .h_code = hash_code_uint64,
    .h_equals = hash_eq_uint64,
    .h_key = buf_id_key_of,
};
static with_spinlock(buf_id_map);

static uint64_t buf_id_alloc(void) {
  static uint64_t nxt_id = 1;
  static with_spinlock(nxt_id);
  uint64_t ret;
  panic_e(lk_acquire(&spinlock_of(nxt_id)));
  ret = nxt_id;
  nxt_id++;
  panic_e(lk_release(&spinlock_of(nxt_id)));
  return ret;
}

struct buffer *buffer_from_id(buf_id_t buf_id) {
  struct linked_node *node = hashmap_get(&buf_id_map, &buf_id);
  if (node == NULL) {
    return NULL;
  }
  struct buffer *buf = CONTAINER_OF(node, struct buffer, buf_id_link);
  return buf;
}

long buffer_alloc(struct part *part, struct buffer **buf, buf_name_t name,
                  msg_size_t max_msg_size, msg_range_t max_nb_msg, que_disc_t que_disc) {
  struct buffer *tmp = kalloc(sizeof(struct buffer));
  memset(tmp, 0, sizeof(struct buffer));
  tmp->buf_id = buf_id_alloc();
  tmp->buf_part_id = part->pa_id;
  tmp->buf_name = kalloc(strlen(name) + 1);
  strcpy(tmp->buf_name, name);
  tmp->buf_que_disc = que_disc;
  tmp->buf_max_msg_size = max_msg_size;
  tmp->buf_max_nb_msg = max_nb_msg;
  tmp->buf_cap = (max_msg_size + sizeof(struct comm_msg)) * max_nb_msg;
  catch_e(part_comm_alloc(part, tmp->buf_cap, &tmp->buf_data), {
    kfree(tmp->buf_name);
    kfree(tmp);
    return err;
  });
  tmp->buf_off_b = 0;
  tmp->buf_off_e = -(max_msg_size + sizeof(struct comm_msg));
  tmp->buf_nb_msg = 0;
  part_add_buf_name(part, tmp);
  spinlock_init(&tmp->buf_lock);
  list_init(&tmp->buf_waiting_procs);
  *buf = tmp;
  panic_e(lk_acquire(&spinlock_of(buf_id_map)));
  hashmap_put(&buf_id_map, &tmp->buf_id_link);
  panic_e(lk_release(&spinlock_of(buf_id_map)));
  return KER_SUCCESS;
}

long buffer_free(struct buffer *buf) {
  // TODO
  return KER_SUCCESS;
}

int buffer_is_full(struct buffer *buf) {
  return buf->buf_nb_msg >= buf->buf_max_nb_msg;
}

int buffer_is_empty(struct buffer *buf) {
  return buf->buf_nb_msg == 0;
}

void buffer_send(struct buffer *buf, msg_addr_t msg_addr, msg_size_t msg_len) {
  assert(!buffer_is_full(buf));
  buf->buf_off_e =
      (buf->buf_off_e + buf->buf_max_msg_size + sizeof(struct comm_msg)) % buf->buf_cap;
  struct comm_msg *msg = (struct comm_msg *)(buf->buf_data + buf->buf_off_e);
  msg->msg_size = msg_len;
  memcpy(msg->msg_data, msg_addr, msg_len);
  buf->buf_nb_msg++;
}

void buffer_recv(struct buffer *buf, msg_addr_t msg_addr, msg_size_t *msg_len) {
  assert(!buffer_is_empty(buf));
  struct comm_msg *msg = (struct comm_msg *)(buf->buf_data + buf->buf_off_b);
  *msg_len = msg->msg_size;
  memcpy(msg_addr, msg->msg_data, *msg_len);
  buf->buf_off_b =
      (buf->buf_off_b + buf->buf_max_msg_size + sizeof(struct comm_msg)) % buf->buf_cap;
  buf->buf_nb_msg--;
}

void buffer_add_waiting_proc(struct buffer *buf, struct proc *proc) {
  list_insert_tail(&buf->buf_waiting_procs, &proc->pr_wait_comm_link);
}

void buffer_del_waiting_proc(struct buffer *buf, struct proc *proc) {
  list_remove_node(&buf->buf_waiting_procs, &proc->pr_wait_comm_link);
}

struct proc *buffer_wakeup_waiting_proc(struct buffer *buf) {
  if (list_empty(&buf->buf_waiting_procs)) {
    return NULL;
  }
  struct proc *proc;
  if (buf->buf_que_disc == FIFO) {
    struct linked_node *node = buf->buf_waiting_procs.l_first;
    proc = CONTAINER_OF(node, struct proc, pr_wait_comm_link);
    list_remove_node(&buf->buf_waiting_procs, node);
    return proc;
  } else {
    struct proc *max_pri_proc = NULL;
    LINKED_NODE_ITER (buf->buf_waiting_procs.l_first, proc, pr_wait_comm_link) {
      if (max_pri_proc == NULL || proc->pr_cur_pri > max_pri_proc->pr_cur_pri) {
        max_pri_proc = proc;
      }
    }
    list_remove_node(&buf->buf_waiting_procs, &max_pri_proc->pr_wait_comm_link);
    return max_pri_proc;
  }
}

wait_range_t buffer_get_waiting_proc_nb(struct buffer *buf) {
  wait_range_t nb = 0;
  struct proc *proc;
  LINKED_NODE_ITER (buf->buf_waiting_procs.l_first, proc, pr_wait_comm_link) {
    nb++;
  }
  return nb;
}

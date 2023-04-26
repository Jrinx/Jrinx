#include <kern/comm/blackboard.h>
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

static const void *bb_id_key_of(const struct linked_node *node) {
  const struct blackboard *bb = CONTAINER_OF(node, struct blackboard, bb_id_link);
  return &bb->bb_id;
}

static struct hlist_head bb_id_map_array[64];
static struct hashmap bb_id_map = {
    .h_array = bb_id_map_array,
    .h_num = 0,
    .h_cap = 64,
    .h_code = hash_code_uint64,
    .h_equals = hash_eq_uint64,
    .h_key = bb_id_key_of,
};
static with_spinlock(bb_id_map);

static uint64_t bb_id_alloc(void) {
  static uint64_t nxt_id = 1;
  static with_spinlock(nxt_id);
  uint64_t ret;
  panic_e(lk_acquire(&spinlock_of(nxt_id)));
  ret = nxt_id;
  nxt_id++;
  panic_e(lk_release(&spinlock_of(nxt_id)));
  return ret;
}

struct blackboard *blackboard_from_id(bb_id_t bb_id) {
  struct linked_node *node = hashmap_get(&bb_id_map, &bb_id);
  if (node == NULL) {
    return NULL;
  }
  struct blackboard *bb = CONTAINER_OF(node, struct blackboard, bb_id_link);
  return bb;
}

long blackboard_alloc(struct part *part, struct blackboard **bb, bb_name_t name,
                      msg_size_t max_msg_size) {
  struct blackboard *tmp = kalloc(sizeof(struct blackboard));
  memset(tmp, 0, sizeof(struct blackboard));
  tmp->bb_id = bb_id_alloc();
  tmp->bb_part_id = part->pa_id;
  tmp->bb_name = kalloc(strlen(name) + 1);
  strcpy(tmp->bb_name, name);
  tmp->bb_max_msg_size = max_msg_size;
  tmp->bb_len = 0;
  tmp->bb_empty_ind = EMPTY;
  catch_e(part_comm_alloc(part, tmp->bb_max_msg_size, &tmp->bb_data), {
    kfree(tmp->bb_name);
    kfree(tmp);
    return err;
  });
  part_add_bb_name(part, tmp);
  spinlock_init(&tmp->bb_lock);
  list_init(&tmp->bb_waiting_procs);
  *bb = tmp;
  panic_e(lk_acquire(&spinlock_of(bb_id_map)));
  hashmap_put(&bb_id_map, &tmp->bb_id_link);
  panic_e(lk_release(&spinlock_of(bb_id_map)));
  return KER_SUCCESS;
}

long blackboard_free(struct blackboard *bb) {
  // TODO
  return KER_SUCCESS;
}

int blackboard_is_full(struct blackboard *bb) {
  return bb->bb_empty_ind == OCCUPIED;
}

int blackboard_is_empty(struct blackboard *bb) {
  return bb->bb_empty_ind == EMPTY;
}

void blackboard_display(struct blackboard *bb, msg_addr_t msg_addr, msg_size_t msg_len) {
  memcpy(bb->bb_data, msg_addr, msg_len);
  bb->bb_len = msg_len;
  bb->bb_empty_ind = OCCUPIED;
}

void blackboard_read(struct blackboard *bb, msg_addr_t msg_addr, msg_size_t *msg_len) {
  memcpy(msg_addr, bb->bb_data, bb->bb_len);
  *msg_len = bb->bb_len;
}

void blackboard_clear(struct blackboard *bb) {
  bb->bb_empty_ind = EMPTY;
}

void blackboard_add_waiting_proc(struct blackboard *bb, struct proc *proc) {
  list_insert_tail(&bb->bb_waiting_procs, &proc->pr_wait_comm_link);
}

void blackboard_del_waiting_proc(struct blackboard *bb, struct proc *proc) {
  list_remove_node(&bb->bb_waiting_procs, &proc->pr_wait_comm_link);
}

struct proc *blackboard_wakeup_waiting_proc(struct blackboard *bb) {
  if (list_empty(&bb->bb_waiting_procs)) {
    return NULL;
  }
  struct linked_node *node = bb->bb_waiting_procs.l_first;
  struct proc *proc = CONTAINER_OF(node, struct proc, pr_wait_comm_link);
  list_remove_node(&bb->bb_waiting_procs, node);
  return proc;
}

wait_range_t blackboard_get_waiting_proc_nb(struct blackboard *bb) {
  wait_range_t nb = 0;
  struct proc *proc;
  LINKED_NODE_ITER (bb->bb_waiting_procs.l_first, proc, pr_wait_comm_link) {
    nb++;
  }
  return nb;
}

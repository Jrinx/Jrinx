#ifndef _LIST_H_
#define _LIST_H_

#include <magicros.h>
#include <stddef.h>

struct linked_node {
  struct linked_node *next;
  struct linked_node **prev;
};

#define LINKED_NODE_NEXT(node, member) ((node)->member.next)

#define LINKED_NODE_ITER(head_node, iter_var, member)                                          \
  for (struct linked_node *iter_var##_node = (head_node);                                      \
       iter_var = CONTAINER_OF(iter_var##_node, typeof(*iter_var), member),                    \
                          iter_var##_node != NULL;                                             \
       iter_var##_node = (iter_var##_node)->next)

struct list_head {
  struct linked_node *l_first;
  struct linked_node **l_last;
};

struct hlist_head {
  struct linked_node *h_first;
};

static inline void list_init(struct list_head *head) {
  head->l_first = NULL;
  head->l_last = &head->l_first;
}

static inline int list_empty(struct list_head *head) {
  return head->l_first == NULL;
}

static inline struct linked_node *list_first(struct list_head *head) {
  return head->l_first;
}

static inline struct linked_node *list_last(struct list_head *head) {
  return head->l_last == &head->l_first ? NULL
                                        : CONTAINER_OF(head->l_last, struct linked_node, next);
}

static inline void list_insert_head(struct list_head *head, struct linked_node *node) {
  node->next = head->l_first;
  if (node->next != NULL) {
    head->l_first->prev = &node->next;
  } else {
    head->l_last = &node->next;
  }
  head->l_first = node;
  node->prev = &head->l_first;
}

static inline void list_insert_tail(struct list_head *head, struct linked_node *node) {
  node->next = NULL;
  node->prev = head->l_last;
  *head->l_last = node;
  head->l_last = &node->next;
}

static inline void list_insert_before(struct linked_node *listnode, struct linked_node *node) {
  node->next = listnode;
  node->prev = listnode->prev;
  *listnode->prev = node;
  listnode->prev = &node->next;
}

static inline void list_remove_node(struct list_head *head, struct linked_node *node) {
  if (node->next != NULL) {
    node->next->prev = node->prev;
  } else {
    head->l_last = node->prev;
  }
  *node->prev = node->next;
}

static inline void hlist_init(struct hlist_head *head) {
  head->h_first = NULL;
}

static inline int hlist_empty(struct hlist_head *head) {
  return head->h_first == NULL;
}

static inline struct linked_node *hlist_first(struct hlist_head *head) {
  return head->h_first;
}

static inline void hlist_insert_head(struct hlist_head *head, struct linked_node *node) {
  node->next = head->h_first;
  if (node->next != NULL) {
    head->h_first->prev = &node->next;
  }
  head->h_first = node;
  node->prev = &head->h_first;
}

static inline void hlist_remove_node(struct linked_node *node) {
  if (node->next != NULL) {
    node->next->prev = node->prev;
  }
  *node->prev = node->next;
}

#endif

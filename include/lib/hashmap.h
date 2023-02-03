#ifndef _LIB_HASHMAP_H_
#define _LIB_HASHMAP_H_

#include <list.h>
#include <stddef.h>

typedef const void *(*hash_key_of_t)(const struct linked_node *node);
typedef unsigned long (*hash_code_func_t)(const void *key);
typedef int (*hash_eq_func_t)(const void *key1, const void *key2);

struct hashmap {
  hash_key_of_t h_key;
  hash_code_func_t h_code;
  hash_eq_func_t h_equals;
  size_t h_cap;
  size_t h_num;
  struct hlist_head *h_array;
};

void hashmap_init(const struct hashmap *map);
void hashmap_put(struct hashmap *map, struct linked_node *node);
void hashmap_remove(struct hashmap *map, const void *key);
struct linked_node *hashmap_get(const struct hashmap *map, const void *key);

unsigned long hash_code_str(const void *key);
int hash_eq_str(const void *key1, const void *key2);
unsigned long hash_code_uint32(const void *key);
int hash_eq_uint32(const void *key1, const void *key2);
unsigned long hash_code_uint64(const void *key);
int hash_eq_uint64(const void *key1, const void *key2);

#define HASHMAP_ALLOC(map, array, cap, key_type, key_of)                                       \
  ({                                                                                           \
    struct hashmap *m = (map);                                                                 \
    m->h_array = (array);                                                                      \
    m->h_num = 0;                                                                              \
    m->h_cap = (cap);                                                                          \
    m->h_code = hash_code_##key_type;                                                          \
    m->h_equals = hash_eq_##key_type;                                                          \
    m->h_key = (key_of);                                                                       \
  })

#endif

#include <lib/hashmap.h>
#include <lib/string.h>
#include <stdint.h>

void hashmap_init(const struct hashmap *map) {
  for (size_t i = 0; i < map->h_cap; i++) {
    hlist_init(map->h_array + i);
  }
}

void hashmap_put(struct hashmap *map, struct linked_node *node) {
  const void *node_key = map->h_key(node);
  unsigned long code = map->h_code(node_key);
  size_t index = code % map->h_cap;
  struct hlist_head *head = &map->h_array[index];
  for (struct linked_node *ptr = head->h_first; ptr != NULL; ptr = ptr->next) {
    if (map->h_equals(node_key, map->h_key(ptr))) {
      hlist_remove_node(ptr);
    }
  }
  hlist_insert_head(head, node);
  map->h_num++;
}

void hashmap_remove(struct hashmap *map, const void *key) {
  unsigned long code = map->h_code(key);
  size_t index = code % map->h_cap;
  struct hlist_head *head = &map->h_array[index];
  for (struct linked_node *ptr = head->h_first; ptr != NULL; ptr = ptr->next) {
    if (map->h_equals(key, map->h_key(ptr))) {
      hlist_remove_node(ptr);
    }
  }
  map->h_num--;
}

struct linked_node *hashmap_get(const struct hashmap *map, const void *key) {
  unsigned long code = map->h_code(key);
  size_t index = code % map->h_cap;
  struct hlist_head *head = &map->h_array[index];
  for (struct linked_node *ptr = head->h_first; ptr != NULL; ptr = ptr->next) {
    if (map->h_equals(key, map->h_key(ptr))) {
      return ptr;
    }
  }
  return NULL;
}

unsigned long hash_code_str(const void *key) {
  const char *str = key;
  const unsigned int mod = 1000000009U;
  unsigned long val = 0;
  unsigned long pow = 1;
  for (size_t i = 0; str[i] != '\0'; i++) {
    val = (val + (str[i] - 'a' + 1) * pow) % mod;
    pow = (pow * 31) % mod;
  }
  return val;
}

int hash_eq_str(const void *key1, const void *key2) {
  const char *str1 = key1;
  const char *str2 = key2;
  return strcmp(str1, str2) == 0;
}

unsigned long hash_code_uint32(const void *key) {
  uint32_t num = *(const uint32_t *)key;
  num = ((num >> 16) ^ num) * 0x119de1f3U;
  num = ((num >> 16) ^ num) * 0x119de1f3U;
  num = (num >> 16) ^ num;
  return num;
}

int hash_eq_uint32(const void *key1, const void *key2) {
  const uint32_t *num1 = key1;
  const uint32_t *num2 = key2;
  return *num1 == *num2;
}

unsigned long hash_code_uint64(const void *key) {
  uint64_t num = *(const uint64_t *)key;
  num = (num ^ (num >> 30)) * 0xbf58476d1ce4e5b9UL;
  num = (num ^ (num >> 27)) * 0x94d049bb133111ebUL;
  num = num ^ (num >> 31);
  return num;
}

int hash_eq_uint64(const void *key1, const void *key2) {
  const uint64_t *num1 = key1;
  const uint64_t *num2 = key2;
  return *num1 == *num2;
}

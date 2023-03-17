#include <aligns.h>
#include <brpred.h>
#include <kern/lib/debug.h>
#include <kern/lock/lock.h>
#include <kern/lock/spinlock.h>
#include <layouts.h>
#include <stddef.h>
#include <stdint.h>

#define KALLOC_MIN_SIZE (sizeof(long) * 4)
#define KALLOC_MAX_SIZE (PGSIZE << 3)

enum buddy_block_state_t {
  FREE = 0,
  USED = 1,
};

struct buddy_block {
  size_t size;
  enum buddy_block_state_t state;
  struct buddy_block *next;
  struct buddy_block *prev;
};

static uint8_t kalloc_pool[KALLOCSIZE] __attribute__((aligned(KALLOC_MAX_SIZE)));
static struct buddy_block *kalloc_block_list = NULL;
static with_spinlock(kalloc_mod);

static struct buddy_block *get_buddy(struct buddy_block *block) {
  if (block->size == KALLOC_MAX_SIZE) {
    return NULL;
  }
  unsigned long addr = (unsigned long)block;
  if (addr % (block->size * 2) == 0) {
    return (struct buddy_block *)(addr + block->size);
  } else {
    return (struct buddy_block *)(addr - block->size);
  }
}

void kalloc_init(void) {
  struct buddy_block *block = (struct buddy_block *)kalloc_pool;
  block->size = KALLOCSIZE;
  block->state = FREE;
  block->next = NULL;
  block->prev = NULL;
  kalloc_block_list = block;
}

size_t kalloc_get_used(void) {
  panic_e(lk_acquire(&spinlock_of(kalloc_mod)));
  unsigned long used = 0;
  struct buddy_block *block = kalloc_block_list;
  while (block != NULL) {
    if (block->state == USED) {
      used += block->size;
    }
    block = block->next;
  }
  panic_e(lk_release(&spinlock_of(kalloc_mod)));
  return used;
}

void *kalloc(size_t size) {
  if (size < KALLOC_MIN_SIZE) {
    size = KALLOC_MIN_SIZE;
  }
  size = align_up(size, sizeof(uint64_t));
  size += sizeof(struct buddy_block);
  if (size > KALLOC_MAX_SIZE) {
    return NULL;
  }
  panic_e(lk_acquire(&spinlock_of(kalloc_mod)));
  struct buddy_block *block = kalloc_block_list;
  while (block != NULL) {
    if (block->state == FREE && block->size >= size) {
      break;
    }
    block = block->next;
  }
  if (block == NULL) {
    panic_e(lk_release(&spinlock_of(kalloc_mod)));
    return NULL;
  }
  while (block->size / 2 >= size) {
    struct buddy_block *new_block = (struct buddy_block *)((uint8_t *)block + block->size / 2);
    new_block->size = block->size / 2;
    new_block->state = FREE;
    new_block->next = block->next;
    new_block->prev = block;
    if (block->next != NULL) {
      block->next->prev = new_block;
    }
    block->next = new_block;
    block->size /= 2;
  }
  block->state = USED;
  panic_e(lk_release(&spinlock_of(kalloc_mod)));
  return (void *)((uint8_t *)block + sizeof(struct buddy_block));
}

void kfree(void *ptr) {
  struct buddy_block *block =
      (struct buddy_block *)((uint8_t *)ptr - sizeof(struct buddy_block));
  panic_e(lk_acquire(&spinlock_of(kalloc_mod)));
  block->state = FREE;
  struct buddy_block *buddy = get_buddy(block);
  while (buddy != NULL && buddy->state == FREE && buddy->size == block->size) {
    struct buddy_block *merged_block = buddy < block ? buddy : block;
    struct buddy_block *other_block = buddy < block ? block : buddy;
    merged_block->size *= 2;
    merged_block->next = other_block->next;
    if (other_block->next != NULL) {
      other_block->next->prev = merged_block;
    }
    block = merged_block;
    buddy = get_buddy(block);
  }
  panic_e(lk_release(&spinlock_of(kalloc_mod)));
}

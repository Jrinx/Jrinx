#include <aligns.h>
#include <brpred.h>
#include <endian.h>
#include <kern/drivers/device.h>
#include <kern/drivers/mems.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lock/lock.h>
#include <kern/lock/spinlock.h>
#include <kern/mm/kalloc.h>
#include <kern/mm/pmm.h>
#include <layouts.h>
#include <lib/string.h>
#include <stddef.h>
#include <stdint.h>

static unsigned long freemem_base = 0;

unsigned long mm_get_freemem_base(void) {
  return freemem_base;
}

void *palloc(size_t size, size_t align) {
  if (unlikely(freemem_base == 0)) {
    extern uint8_t kern_end[];
    freemem_base = (unsigned long)kern_end;
  }
  freemem_base = align_up(freemem_base, align);
  void *res = (void *)freemem_base;
  freemem_base += size;
  return res;
}

static struct list_head *pf_free_list;
static with_spinlock(pf_free_list);
static struct phy_frame **pf_array;
static size_t *pf_array_len;

static long __attribute__((warn_unused_result)) pa2sel(unsigned long addr, unsigned long *sel) {
  size_t mem_num = mem_get_num();
  for (size_t i = 0; i < mem_num; i++) {
    uint64_t mem_addr;
    uint64_t mem_size;
    catch_e(mem_get_addr(i, &mem_addr));
    catch_e(mem_get_size(i, &mem_size));
    if (addr >= mem_addr && addr < mem_addr + mem_size) {
      *sel = i;
      return KER_SUCCESS;
    }
  }
  return -KER_MEM_ER;
}

static long __attribute__((warn_unused_result))
frame2sel(struct phy_frame *frame, unsigned long *sel) {
  size_t mem_num = mem_get_num();
  for (size_t i = 0; i < mem_num; i++) {
    if (frame >= pf_array[i] && frame < pf_array[i] + pf_array_len[i]) {
      *sel = i;
      return KER_SUCCESS;
    }
  }
  return -KER_MEM_ER;
}

void pmm_init(void) {
  size_t mem_num = mem_get_num();
  pf_free_list = kalloc(sizeof(struct list_head) * mem_num);
  pf_array = kalloc(sizeof(struct phy_frame *) * mem_num);
  pf_array_len = kalloc(sizeof(size_t) * mem_num);
  for (size_t i = 0; i < mem_num; i++) {
    list_init(&pf_free_list[i]);
    uint64_t mem_size;
    panic_e(mem_get_size(i, &mem_size));
    pf_array_len[i] = mem_size / PGSIZE;
    pf_array[i] = palloc(sizeof(struct phy_frame) * pf_array_len[i], PGSIZE);
    for (size_t j = 0; j < pf_array_len[i]; j++) {
      pf_array[i][j].pf_ref = 0;
      list_insert_tail(&pf_free_list[i], &pf_array[i][j].pf_link);
    }
  }

  freemem_base = align_up(freemem_base, PGSIZE);

  for (unsigned long rsvaddr = SBIBASE; rsvaddr < freemem_base; rsvaddr += PGSIZE) {
    struct phy_frame *frame;
    panic_e(pa2frame(rsvaddr, &frame));
    unsigned long sel;
    panic_e(pa2sel(rsvaddr, &sel));
    list_remove_node(&pf_free_list[sel], &frame->pf_link);
    frame->pf_ref = 1;
  }
}

long pa2frame(unsigned long addr, struct phy_frame **frame) {
  unsigned long sel;
  catch_e(pa2sel(addr, &sel));
  uint64_t mem_addr;
  catch_e(mem_get_addr(sel, &mem_addr));
  unsigned long off = addr - mem_addr;
  *frame = &pf_array[sel][off / PGSIZE];
  return KER_SUCCESS;
}

long frame2pa(struct phy_frame *frame, unsigned long *addr) {
  unsigned long sel;
  catch_e(frame2sel(frame, &sel));
  unsigned long off = (frame - pf_array[sel]) * PGSIZE;
  uint64_t mem_addr;
  catch_e(mem_get_addr(sel, &mem_addr));
  *addr = mem_addr + off;
  return KER_SUCCESS;
}

long phy_frame_alloc(struct phy_frame **frame) {
  size_t mem_num = mem_get_num();
  panic_e(lk_acquire(&spinlock_of(pf_free_list)));
  for (size_t i = 0; i < mem_num; i++) {
    if (!list_empty(&pf_free_list[i])) {
      struct linked_node *first = pf_free_list[i].l_first;
      list_remove_node(&pf_free_list[i], first);
      *frame = CONTAINER_OF(first, struct phy_frame, pf_link);
      unsigned long pa;
      panic_e(frame2pa(*frame, &pa));
      memset((void *)pa, 0, PGSIZE);
      panic_e(lk_release(&spinlock_of(pf_free_list)));
      return KER_SUCCESS;
    }
  }
  panic_e(lk_release(&spinlock_of(pf_free_list)));
  return -KER_MEM_ER;
}

static long phy_frame_free(struct phy_frame *frame) {
  unsigned long sel;
  catch_e(frame2sel(frame, &sel));
  panic_e(lk_acquire(&spinlock_of(pf_free_list)));
  list_insert_tail(&pf_free_list[sel], &frame->pf_link);
  panic_e(lk_release(&spinlock_of(pf_free_list)));
  return KER_SUCCESS;
}

static with_spinlock(frame_ref_mod);

long phy_frame_ref_dec(struct phy_frame *frame) {
  panic_e(lk_acquire(&spinlock_of(frame_ref_mod)));
  if (frame->pf_ref == 0) {
    panic_e(lk_release(&spinlock_of(frame_ref_mod)));
    return -KER_MEM_ER;
  }
  frame->pf_ref--;
  if (frame->pf_ref == 0) {
    catch_e(phy_frame_free(frame), {
      panic_e(lk_release(&spinlock_of(frame_ref_mod)));
      return err;
    });
  }
  panic_e(lk_release(&spinlock_of(frame_ref_mod)));
  return KER_SUCCESS;
}

long phy_frame_ref_inc(struct phy_frame *frame) {
  panic_e(lk_acquire(&spinlock_of(frame_ref_mod)));
  frame->pf_ref++;
  panic_e(lk_release(&spinlock_of(frame_ref_mod)));
  return KER_SUCCESS;
}

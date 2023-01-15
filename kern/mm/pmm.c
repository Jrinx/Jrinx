#include <aligns.h>
#include <brpred.h>
#include <endian.h>
#include <kern/drivers/device.h>
#include <kern/drivers/mems.h>
#include <kern/lib/debug.h>
#include <kern/lib/errors.h>
#include <kern/lock/lock.h>
#include <kern/lock/spinlock.h>
#include <kern/mm/pmm.h>
#include <layouts.h>
#include <lib/string.h>
#include <stddef.h>
#include <stdint.h>

static unsigned long freemem_base = 0;

unsigned long mm_get_freemem_base(void) {
  return freemem_base;
}

static void *bare_alloc(size_t size, size_t align) {
  if (unlikely(freemem_base == 0)) {
    extern uint8_t kern_end[];
    freemem_base = (unsigned long)kern_end;
  }

  freemem_base = align_up(freemem_base, align);

  void *res = (void *)freemem_base;

  freemem_base += size;

  return res;
}

static void bare_free(const void *ptr) {
  UNIMPLEMENTED;
}

static void mem_print_bytes(unsigned long size, size_t shift) {
  static const char *unit[] = {"B", "KiB", "MiB", "GiB", "TiB"};

  if (size == 0 && shift == 0) {
    printk("0");
    return;
  }

  unsigned long rem = size & ((1UL << 10) - 1);
  unsigned long quo = size >> 10;
  if (quo != 0) {
    mem_print_bytes(quo, shift + 1);
  }
  if (rem != 0) {
    if (quo != 0) {
      printk(" + ");
    }
    printk("%lu %s", rem, unit[shift]);
  }
}

void mem_print_range(unsigned long addr, unsigned long size, const char *suffix) {
  printk("[%016lx, %016lx) <size: ", addr, addr + size);
  mem_print_bytes(size, 0);
  printk(">");
  if (suffix) {
    printk("%s", suffix);
  } else {
    printk("\n");
  }
}

void *(*alloc)(size_t size, size_t align) = bare_alloc;
void (*free)(const void *ptr) = bare_free;

static struct phy_frame_list *pf_free_list;
static with_spinlock(pf_free_list);
static struct phy_frame **pf_array;
static size_t *pf_array_len;

void pmm_init(void) {
  size_t mem_num = mem_get_num();
  pf_free_list = alloc(sizeof(struct phy_frame_list) * mem_num, sizeof(struct phy_frame_list));
  pf_array = alloc(sizeof(struct phy_frame *) * mem_num, sizeof(struct phy_frame *));
  pf_array_len = alloc(sizeof(size_t) * mem_num, sizeof(size_t));
  for (size_t i = 0; i < mem_num; i++) {
    LIST_INIT(&pf_free_list[i]);
    uint64_t mem_size;
    panic_e(mem_get_size(i, &mem_size));
    pf_array_len[i] = mem_size / PGSIZE;
    pf_array[i] = alloc(sizeof(struct phy_frame) * pf_array_len[i],
                        PGSIZE >= sizeof(struct phy_frame) ? PGSIZE : sizeof(struct phy_frame));
    for (size_t j = 0; j < pf_array_len[i]; j++) {
      pf_array[i][j].pf_ref = 0;
      LIST_INSERT_HEAD(&pf_free_list[i], &pf_array[i][j], pf_link);
    }
  }

  freemem_base = align_up(freemem_base, PGSIZE);

  for (unsigned long rsvaddr = SBIBASE; rsvaddr < freemem_base; rsvaddr += PGSIZE) {
    struct phy_frame *frame;
    panic_e(pa2frame(rsvaddr, &frame));
    LIST_REMOVE(frame, pf_link);
    frame->pf_ref = 1;
  }
}

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
  catch_e(lk_acquire(&spinlock_of(pf_free_list)));
  for (size_t i = 0; i < mem_num; i++) {
    if (!LIST_EMPTY(&pf_free_list[i])) {
      *frame = LIST_FIRST(&pf_free_list[i]);
      LIST_REMOVE(*frame, pf_link);
      unsigned long pa;
      panic_e(frame2pa(*frame, &pa));
      memset((void *)pa, 0, PGSIZE);
      catch_e(lk_release(&spinlock_of(pf_free_list)));
      return KER_SUCCESS;
    }
  }
  catch_e(lk_release(&spinlock_of(pf_free_list)));
  return -KER_MEM_ER;
}

static long phy_frame_free(struct phy_frame *frame) {
  unsigned long sel;
  catch_e(frame2sel(frame, &sel));
  catch_e(lk_acquire(&spinlock_of(pf_free_list)));
  LIST_INSERT_HEAD(&pf_free_list[sel], frame, pf_link);
  catch_e(lk_release(&spinlock_of(pf_free_list)));
  return KER_SUCCESS;
}

static with_spinlock(frame_ref_mod);

long phy_frame_ref_dec(struct phy_frame *frame) {
  catch_e(lk_acquire(&spinlock_of(frame_ref_mod)));
  if (frame->pf_ref == 0) {
    catch_e(lk_release(&spinlock_of(frame_ref_mod)));
    return -KER_MEM_ER;
  }
  frame->pf_ref--;
  if (frame->pf_ref == 0) {
    catch_e(phy_frame_free(frame), {
      catch_e(lk_release(&spinlock_of(frame_ref_mod)));
      return err;
    });
  }
  catch_e(lk_release(&spinlock_of(frame_ref_mod)));
  return KER_SUCCESS;
}

long phy_frame_ref_inc(struct phy_frame *frame) {
  catch_e(lk_acquire(&spinlock_of(frame_ref_mod)));
  frame->pf_ref++;
  catch_e(lk_release(&spinlock_of(frame_ref_mod)));
  return KER_SUCCESS;
}

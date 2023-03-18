#include <bitmap.h>
#include <brpred.h>
#include <kern/drivers/cpus.h>
#include <kern/lib/debug.h>
#include <kern/lib/regs.h>
#include <kern/lib/sync.h>
#include <kern/lock/lock.h>
#include <kern/lock/spinlock.h>
#include <kern/mm/kalloc.h>
#include <lib/string.h>

static unsigned long *cpus_asid_max;
static uint64_t *cpus_asid_generation;
static unsigned long **cpus_asid_bitmap;
static size_t *cpus_asid_bitmap_size;
static struct lock *cpus_asid_bitmap_lock;

void asid_array_init(void) {
  cpus_asid_max = kalloc(sizeof(unsigned long) * cpus_get_count());
  cpus_asid_generation = kalloc(sizeof(uint64_t) * cpus_get_count());
  cpus_asid_bitmap = kalloc(sizeof(unsigned long *) * cpus_get_count());
  cpus_asid_bitmap_size = kalloc(sizeof(size_t) * cpus_get_count());
  cpus_asid_bitmap_lock = kalloc(sizeof(struct lock) * cpus_get_count());
}

void asid_init(void) {
  rv64_satp bak_satp = {.val = csrr_satp()};
  rv64_satp asid_probe_satp = {
      .bits = {.mode = bak_satp.bits.mode, .asid = 0xffffU, .ppn = bak_satp.bits.ppn}};
  csrw_satp(asid_probe_satp.val);
  asid_probe_satp.val = csrr_satp();
  cpus_asid_max[hrt_get_id()] = asid_probe_satp.bits.asid;
  cpus_asid_generation[hrt_get_id()] = 1;
  info("asid in cpu@%lu impl probed [0, %lu]\n", hrt_get_id(), cpus_asid_max[hrt_get_id()]);
  cpus_asid_bitmap_size[hrt_get_id()] = BITMAP_SIZE(cpus_asid_max[hrt_get_id()] + 1);
  cpus_asid_bitmap[hrt_get_id()] =
      kalloc(sizeof(unsigned long) * cpus_asid_bitmap_size[hrt_get_id()]);
  memset(cpus_asid_bitmap[hrt_get_id()], 0,
         sizeof(unsigned long) * cpus_asid_bitmap_size[hrt_get_id()]);
  csrw_satp(bak_satp.val);
  sfence_vma;
  spinlock_init(&cpus_asid_bitmap_lock[hrt_get_id()]);
}

unsigned long asid_get_max(void) {
  return cpus_asid_max[hrt_get_id()];
}

uint64_t asid_get_generation(void) {
  return cpus_asid_generation[hrt_get_id()];
}

void asid_inc_generation(void) {
  cpus_asid_generation[hrt_get_id()]++;
  memset(cpus_asid_bitmap[hrt_get_id()], 0,
         sizeof(unsigned long) * cpus_asid_bitmap_size[hrt_get_id()]);
}

long asid_alloc(unsigned long *asid) {
  panic_e(lk_acquire(&cpus_asid_bitmap_lock[hrt_get_id()]));
  unsigned bit = bitmap_find_first_zero_bit(cpus_asid_bitmap[hrt_get_id()],
                                            cpus_asid_bitmap_size[hrt_get_id()]);
  if (bit > cpus_asid_max[hrt_get_id()]) {
    panic_e(lk_release(&cpus_asid_bitmap_lock[hrt_get_id()]));
    return -KER_ASID_ER;
  }
  bitmap_set_bit(cpus_asid_bitmap[hrt_get_id()], bit);
  panic_e(lk_release(&cpus_asid_bitmap_lock[hrt_get_id()]));
  *asid = bit;
  return KER_SUCCESS;
}

long asid_free(unsigned long asid) {
  panic_e(lk_acquire(&cpus_asid_bitmap_lock[hrt_get_id()]));
  if (asid > cpus_asid_max[hrt_get_id()] ||
      unlikely(!bitmap_get_bit(cpus_asid_bitmap[hrt_get_id()], asid))) {
    panic_e(lk_release(&cpus_asid_bitmap_lock[hrt_get_id()]));
    return -KER_ASID_ER;
  }
  bitmap_clr_bit(cpus_asid_bitmap[hrt_get_id()], asid);
  panic_e(lk_release(&cpus_asid_bitmap_lock[hrt_get_id()]));
  return KER_SUCCESS;
}

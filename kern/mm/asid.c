#include <bitmap.h>
#include <brpred.h>
#include <kern/drivers/cpus.h>
#include <kern/lib/debug.h>
#include <kern/lib/regs.h>
#include <kern/lock/lock.h>
#include <kern/lock/spinlock.h>
#include <kern/mm/pmm.h>

static unsigned long *cpus_asid_max;
static unsigned long **cpus_asid_bitmap;
static size_t *cpus_asid_bitmap_size;
static struct lock *cpus_asid_bitmap_lock;

void asid_init(void) {
  cpus_asid_max = alloc(sizeof(unsigned long) * cpus_get_count(), sizeof(unsigned long));
  cpus_asid_bitmap = alloc(sizeof(unsigned long *) * cpus_get_count(), sizeof(unsigned long *));
  cpus_asid_bitmap_size = alloc(sizeof(size_t) * cpus_get_count(), sizeof(size_t));
  cpus_asid_bitmap_lock = alloc(sizeof(struct lock) * cpus_get_count(), sizeof(struct lock));
  for (unsigned long i = 0; i < cpus_get_count(); i++) {
    rv64_satp bak_satp = {.val = csrr_satp()};
    rv64_satp asid_probe_satp = {.bits = {.mode = BARE, .asid = 0xffffU, .ppn = 0UL}};
    csrw_satp(asid_probe_satp.val);
    asid_probe_satp.val = csrr_satp();
    cpus_asid_max[i] = asid_probe_satp.bits.asid;
    info("asid in cpu@%lu impl probed [0, %lu]\n", i, cpus_asid_max[i]);
    cpus_asid_bitmap_size[i] = BITMAP_SIZE(cpus_asid_max[i] + 1);
    cpus_asid_bitmap[i] =
        alloc(sizeof(unsigned long) * cpus_asid_bitmap_size[i], sizeof(unsigned long));
    csrw_satp(bak_satp.val);
    spinlock_init(&cpus_asid_bitmap_lock[i]);
  }
}

unsigned long asid_get_max(void) {
  return cpus_asid_max[hrt_get_id()];
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

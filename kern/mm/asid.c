#include <bitmap.h>
#include <brpred.h>
#include <kern/lib/debug.h>
#include <kern/lib/regs.h>
#include <kern/lock/lock.h>
#include <kern/lock/spinlock.h>
#include <kern/mm/pmm.h>

unsigned long asid_max;
static unsigned long *asid_bitmap;
static size_t asid_bitmap_size;
static with_spinlock(asid_bitmap);

void asid_init(void) {
  rv64_satp bak_satp = {.val = csrr_satp()};
  rv64_satp asid_probe_satp = {.bits = {.mode = BARE, .asid = 0xffffU, .ppn = 0UL}};
  csrw_satp(asid_probe_satp.val);
  asid_probe_satp.val = csrr_satp();
  asid_max = asid_probe_satp.bits.asid;
  info("asid impl probed [0, %lu]\n", asid_max);
  asid_bitmap_size = BITMAP_SIZE(asid_max + 1);
  asid_bitmap = alloc(sizeof(unsigned long) * asid_bitmap_size, sizeof(unsigned long));
  csrw_satp(bak_satp.val);
}

long asid_alloc(unsigned long *asid) {
  panic_e(lk_acquire(&spinlock_of(asid_bitmap)));
  unsigned bit = bitmap_find_first_zero_bit(asid_bitmap, asid_bitmap_size);
  if (bit > asid_max) {
    panic_e(lk_release(&spinlock_of(asid_bitmap)));
    return -KER_ASID_ER;
  }
  bitmap_set_bit(asid_bitmap, bit);
  panic_e(lk_release(&spinlock_of(asid_bitmap)));
  *asid = bit;
  return KER_SUCCESS;
}

long asid_free(unsigned long asid) {
  panic_e(lk_acquire(&spinlock_of(asid_bitmap)));
  if (asid > asid_max || unlikely(!bitmap_get_bit(asid_bitmap, asid))) {
    panic_e(lk_release(&spinlock_of(asid_bitmap)));
    return -KER_ASID_ER;
  }
  bitmap_clr_bit(asid_bitmap, asid);
  panic_e(lk_release(&spinlock_of(asid_bitmap)));
  return KER_SUCCESS;
}

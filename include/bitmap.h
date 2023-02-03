#ifndef _BITMAP_H_
#define _BITMAP_H_

#include <bitset.h>
#include <stddef.h>

#define BIT_MASK(i) (1UL << ((i) % (sizeof(unsigned long) * 8)))
#define BIT_WORD(i) ((i) / (sizeof(unsigned long) * 8))

#define BITMAP_SIZE(nr) (((nr)-1) / (sizeof(unsigned long) * 8) + 1)
#define BITMAP_DECL(ident, nr) unsigned long ident[BITMAP_SIZE(nr)]

static inline void bitmap_set_bit(unsigned long *bitmap, unsigned long i) {
  bitmap[BIT_WORD(i)] |= BIT_MASK(i);
}

static inline void bitmap_clr_bit(unsigned long *bitmap, unsigned long i) {
  bitmap[BIT_WORD(i)] &= ~BIT_MASK(i);
}

static inline unsigned bitmap_get_bit(unsigned long *bitmap, unsigned long i) {
  return (bitmap[BIT_WORD(i)] >> (i % (sizeof(unsigned long) * 8))) & 1UL;
}

static inline unsigned bitmap_find_first_zero_bit(unsigned long *bitmap, size_t nr) {
  size_t i;
  for (i = 0; i < nr && !~bitmap[i]; i++) {
  }

  unsigned j = ffs(~bitmap[i]);
  return j ? i * (sizeof(unsigned long) * 8) + (j - 1) : -1UL;
}

#endif

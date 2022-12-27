#ifndef _ENDIAN_H_
#define _ENDIAN_H_

#include <stdint.h>

#define LITTLE_ENDIAN 0
#define BIG_ENDIAN 1

static inline uint32_t bswap32(uint32_t x) {
  unsigned char *b = (unsigned char *)&x;
  return (uint32_t)b[0] << 24 | (uint32_t)b[1] << 16 | (uint32_t)b[2] << 8 | (uint32_t)b[3];
}

static inline uint64_t bswap64(uint64_t x) {
  unsigned char *b = (unsigned char *)&x;
  return (uint64_t)b[0] << 56 | (uint64_t)b[1] << 48 | (uint64_t)b[2] << 40 |
         (uint64_t)b[3] << 32 | (uint64_t)b[4] << 24 | (uint64_t)b[5] << 16 |
         (uint64_t)b[6] << 8 | (uint64_t)b[7];
}

#if CONFIG_ENDIAN == LITTLE_ENDIAN
#define from_be(x) _Generic((x), uint32_t : bswap32, uint64_t : bswap64)((x))
#else
#define from_be(x) (x)
#endif

#if CONFIG_ENDIAN == LITTLE_ENDIAN
#define from_le(x) (x)
#else
#define from_le(x) _Generic((x), uint32_t : bswap32, uint64_t : bswap64)((x))
#endif

#endif

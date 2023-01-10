#include <stddef.h>
#include <stdint.h>

void *memcpy(void *restrict dst, const void *restrict src, size_t n) {
  void *dstaddr = dst;
  void *max = dst + n;

  if (((unsigned long)src % sizeof(uint32_t)) != ((unsigned long)dst % sizeof(uint32_t))) {
    while (dst < max) {
      *(uint8_t *)dst++ = *(uint8_t *)src++;
    }
    return dstaddr;
  }

  while (((unsigned long)dst % sizeof(uint32_t)) && dst < max) {
    *(uint8_t *)dst++ = *(uint8_t *)src++;
  }

  while (dst + sizeof(uint32_t) <= max) {
    *(uint32_t *)dst = *(uint32_t *)src;
    dst += 4;
    src += 4;
  }

  while (dst < max) {
    *(uint8_t *)dst++ = *(uint8_t *)src++;
  }
  return dstaddr;
}

void *memset(void *restrict dst, int c, size_t n) {
  void *dstaddr = dst;
  void *max = dst + n;
  uint8_t byte = (uint8_t)(c & 0xff);
  uint32_t word = byte | byte << 8 | byte << 16 | byte << 24;

  while (((unsigned long)dst % sizeof(uint32_t)) && dst < max) {
    *(uint8_t *)dst++ = byte;
  }

  while (dst + 4 <= max) {
    *(uint32_t *)dst = word;
    dst += 4;
  }

  while (dst < max) {
    *(uint8_t *)dst++ = byte;
  }
  return dstaddr;
}

size_t strlen(const char *s) {
  int n;

  for (n = 0; *s; s++) {
    n++;
  }

  return n;
}

char *strcpy(char *restrict dst, const char *restrict src) {
  char *ret = dst;

  while ((*dst++ = *src++) != 0) {
  }

  return ret;
}

const char *strchr(const char *restrict s, int c) {
  for (; *s; s++) {
    if (*s == c) {
      return s;
    }
  }
  return 0;
}

int strcmp(const char *restrict p, const char *restrict q) {
  while (*p && *p == *q) {
    p++, q++;
  }

  if ((char)*p < (char)*q) {
    return -1;
  }

  if ((char)*p > (char)*q) {
    return 1;
  }

  return 0;
}

static long strtol(const char *str) {
  long sign = 1L;
  long res = 0;
  for (; *str == '-' || *str == '+'; str++) {
    sign = *str == '-' ? -sign : sign;
  }
  for (; *str; str++) {
    res = res * 10 + *str - '0';
  }
  return sign * res;
}

int atoi(const char *str) {
  return (int)strtol(str);
}

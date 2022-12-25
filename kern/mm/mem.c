#include <brpred.h>
#include <stddef.h>
#include <stdint.h>

static unsigned long freemem_base = 0;

void *bare_alloc(size_t size) {
  if (unlikely(freemem_base == 0)) {
    extern uint8_t kern_end[];
    freemem_base = (unsigned long)kern_end;
  }

  freemem_base = freemem_base / size * size + size;

  void *res = (void *)freemem_base;

  freemem_base += size;

  return res;
}

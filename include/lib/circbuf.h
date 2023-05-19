#ifndef _LIB_CIRCBUF_H_
#define _LIB_CIRCBUF_H_

#include <stddef.h>
#include <stdint.h>

struct circbuf {
  void *cc_buf;
  size_t cc_ele_size;
  size_t cc_cap;
  size_t cc_cnt;
  uintptr_t cc_head;
  uintptr_t cc_tail;
};

void circbuf_init(struct circbuf *cc, void *buf, size_t ele_size, size_t cap);
void circbuf_reset(struct circbuf *cc);
int circbuf_is_full(struct circbuf *cc);
int circbuf_is_empty(struct circbuf *cc);
uintptr_t circbuf_enqu_st(struct circbuf *cc);
void circbuf_enqu_ed(struct circbuf *cc);
uintptr_t circbuf_dequ_st(struct circbuf *cc);
void circbuf_dequ_ed(struct circbuf *cc);
void circbuf_enqueue(struct circbuf *cc, void *ele);
void circbuf_dequeue(struct circbuf *cc, void *ele);

#endif

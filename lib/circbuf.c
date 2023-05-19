#include <lib/circbuf.h>
#include <lib/string.h>

void circbuf_init(struct circbuf *cc, void *buf, size_t ele_size, size_t cap) {
  cc->cc_buf = buf;
  cc->cc_ele_size = ele_size;
  cc->cc_cap = cap;
  cc->cc_head = 0;
  cc->cc_tail = -ele_size;
  cc->cc_cnt = 0;
}

void circbuf_reset(struct circbuf *cc) {
  cc->cc_head = 0;
  cc->cc_tail = -cc->cc_ele_size;
  cc->cc_cnt = 0;
}

int circbuf_is_full(struct circbuf *cc) {
  return cc->cc_cnt == cc->cc_cap;
}

int circbuf_is_empty(struct circbuf *cc) {
  return cc->cc_cnt == 0;
}

uintptr_t circbuf_enqu_st(struct circbuf *cc) {
  cc->cc_tail = (cc->cc_tail + cc->cc_ele_size) % (cc->cc_cap * cc->cc_ele_size);
  return cc->cc_tail;
}

void circbuf_enqu_ed(struct circbuf *cc) {
  cc->cc_cnt++;
}

uintptr_t circbuf_dequ_st(struct circbuf *cc) {
  cc->cc_cnt--;
  return cc->cc_head;
}

void circbuf_dequ_ed(struct circbuf *cc) {
  cc->cc_head = (cc->cc_head + cc->cc_ele_size) % (cc->cc_cap * cc->cc_ele_size);
}

void circbuf_enqueue(struct circbuf *cc, void *ele) {
  circbuf_enqu_st(cc);
  memcpy(cc->cc_buf + cc->cc_tail, ele, cc->cc_ele_size);
  circbuf_enqu_ed(cc);
}

void circbuf_dequeue(struct circbuf *cc, void *ele) {
  circbuf_dequ_st(cc);
  memcpy(ele, cc->cc_buf + cc->cc_head, cc->cc_ele_size);
  circbuf_dequ_ed(cc);
}

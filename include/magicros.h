#ifndef _MAGICROS_H_
#define _MAGICROS_H_

#define _MACRO_CONCAT(x, y) x##y
#define MACRO_CONCAT(x, y) _MACRO_CONCAT(x, y)

#define _9TH_ARGS(a1, a2, a3, a4, a5, a6, a7, a8, a9, ...) a9
#define VA_CNT(...) _9TH_ARGS(_magic, ##__VA_ARGS__, 7, 6, 5, 4, 3, 2, 1, 0)

#define _CURRYING_HEAD(...) (__VA_ARGS__
#define _CURRYING_TAIL(...) , ##__VA_ARGS__)
#define CURRYING(func, ...) (func) _CURRYING_HEAD(__VA_ARGS__) _CURRYING_TAIL

#define UNIMPLEMENTED __builtin_unreachable()

#define OFFSET_OF(st, mb) ((unsigned long)(&((st *)0)->mb))
#define CONTAINER_OF(ptr, st, mb)                                                              \
  ({                                                                                           \
    const typeof(((st *)0)->mb) *_ptr = (ptr);                                                 \
    (st *)((char *)_ptr - OFFSET_OF(st, mb));                                                  \
  })

#endif

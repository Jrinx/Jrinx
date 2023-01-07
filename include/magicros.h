#ifndef _MAGICROS_H_
#define _MAGICROS_H_

#define _MACRO_CONCAT(x, y) x##y
#define MACRO_CONCAT(x, y) _MACRO_CONCAT(x, y)

#define _9TH_ARGS(a1, a2, a3, a4, a5, a6, a7, a8, a9, ...) a9
#define VA_CNT(...) _9TH_ARGS(_magic, ##__VA_ARGS__, 7, 6, 5, 4, 3, 2, 1, 0)

#define _CURRYING_1N_HEAD(head) ((head)
#define _CURRYING_1N_TAIL(...) , ##__VA_ARGS__)
#define CURRYING_1N(func, head) (func) _CURRYING_1N_HEAD(head) _CURRYING_1N_TAIL

#ifdef fatal
#define UNIMPLEMENTED fatal("Unimplemented!")
#endif

#endif

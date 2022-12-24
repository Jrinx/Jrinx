#ifndef _BRPRED_H_
#define _BRPRED_H_

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect((x), 0)

#endif

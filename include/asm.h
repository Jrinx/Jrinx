#ifndef _ASM_H_
#define _ASM_H_

#define EXPORT(symbol)                                                                         \
  .globl symbol;                                                                               \
  symbol:

#endif

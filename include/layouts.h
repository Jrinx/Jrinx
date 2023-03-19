#ifndef _LAYOUTS_H_
#define _LAYOUTS_H_

#define SYSCORE 1

#define PGSHIFT 12
#define PGSIZE (1 << PGSHIFT)

#define SBIBASE 0x80000000

#define KERNOFF 0x200000
#define KERNBASE (SBIBASE + KERNOFF)
#define KERNENTRY KERNBASE

#define KSTKSHIFT 4
#define KSTKSIZE (PGSIZE << KSTKSHIFT)

#define KALLOCSIZE (PGSIZE << 7)

#define MMIOBASE 0xffffffC000000000

#define USTKLIMIT 0x3000000000
#define UMAINSTKTOP_SYM _main_proc_stacktop

#define USERBASE 0x400000

#endif

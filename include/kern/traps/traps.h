#ifndef _KERN_TRAPS_TRAPS_H_
#define _KERN_TRAPS_TRAPS_H_

#include <kern/lib/regs.h>

#define PT_REG0 (0)
#define PT_REG1 (PT_REG0 + RV_XLEN)
#define PT_REG2 (PT_REG1 + RV_XLEN)
#define PT_REG3 (PT_REG2 + RV_XLEN)
#define PT_REG4 (PT_REG3 + RV_XLEN)
#define PT_REG5 (PT_REG4 + RV_XLEN)
#define PT_REG6 (PT_REG5 + RV_XLEN)
#define PT_REG7 (PT_REG6 + RV_XLEN)
#define PT_REG8 (PT_REG7 + RV_XLEN)
#define PT_REG9 (PT_REG8 + RV_XLEN)
#define PT_REG10 (PT_REG9 + RV_XLEN)
#define PT_REG11 (PT_REG10 + RV_XLEN)
#define PT_REG12 (PT_REG11 + RV_XLEN)
#define PT_REG13 (PT_REG12 + RV_XLEN)
#define PT_REG14 (PT_REG13 + RV_XLEN)
#define PT_REG15 (PT_REG14 + RV_XLEN)
#define PT_REG16 (PT_REG15 + RV_XLEN)
#define PT_REG17 (PT_REG16 + RV_XLEN)
#define PT_REG18 (PT_REG17 + RV_XLEN)
#define PT_REG19 (PT_REG18 + RV_XLEN)
#define PT_REG20 (PT_REG19 + RV_XLEN)
#define PT_REG21 (PT_REG20 + RV_XLEN)
#define PT_REG22 (PT_REG21 + RV_XLEN)
#define PT_REG23 (PT_REG22 + RV_XLEN)
#define PT_REG24 (PT_REG23 + RV_XLEN)
#define PT_REG25 (PT_REG24 + RV_XLEN)
#define PT_REG26 (PT_REG25 + RV_XLEN)
#define PT_REG27 (PT_REG26 + RV_XLEN)
#define PT_REG28 (PT_REG27 + RV_XLEN)
#define PT_REG29 (PT_REG28 + RV_XLEN)
#define PT_REG30 (PT_REG29 + RV_XLEN)
#define PT_REG31 (PT_REG30 + RV_XLEN)

#define PT_FREG0 (PT_REG31 + RV_XLEN)
#define PT_FREG1 (PT_FREG0 + RV_XLEN)
#define PT_FREG2 (PT_FREG1 + RV_XLEN)
#define PT_FREG3 (PT_FREG2 + RV_XLEN)
#define PT_FREG4 (PT_FREG3 + RV_XLEN)
#define PT_FREG5 (PT_FREG4 + RV_XLEN)
#define PT_FREG6 (PT_FREG5 + RV_XLEN)
#define PT_FREG7 (PT_FREG6 + RV_XLEN)
#define PT_FREG8 (PT_FREG7 + RV_XLEN)
#define PT_FREG9 (PT_FREG8 + RV_XLEN)
#define PT_FREG10 (PT_FREG9 + RV_XLEN)
#define PT_FREG11 (PT_FREG10 + RV_XLEN)
#define PT_FREG12 (PT_FREG11 + RV_XLEN)
#define PT_FREG13 (PT_FREG12 + RV_XLEN)
#define PT_FREG14 (PT_FREG13 + RV_XLEN)
#define PT_FREG15 (PT_FREG14 + RV_XLEN)
#define PT_FREG16 (PT_FREG15 + RV_XLEN)
#define PT_FREG17 (PT_FREG16 + RV_XLEN)
#define PT_FREG18 (PT_FREG17 + RV_XLEN)
#define PT_FREG19 (PT_FREG18 + RV_XLEN)
#define PT_FREG20 (PT_FREG19 + RV_XLEN)
#define PT_FREG21 (PT_FREG20 + RV_XLEN)
#define PT_FREG22 (PT_FREG21 + RV_XLEN)
#define PT_FREG23 (PT_FREG22 + RV_XLEN)
#define PT_FREG24 (PT_FREG23 + RV_XLEN)
#define PT_FREG25 (PT_FREG24 + RV_XLEN)
#define PT_FREG26 (PT_FREG25 + RV_XLEN)
#define PT_FREG27 (PT_FREG26 + RV_XLEN)
#define PT_FREG28 (PT_FREG27 + RV_XLEN)
#define PT_FREG29 (PT_FREG28 + RV_XLEN)
#define PT_FREG30 (PT_FREG29 + RV_XLEN)
#define PT_FREG31 (PT_FREG30 + RV_XLEN)

#define PT_SSTATUS (PT_FREG31 + RV_XLEN)
#define PT_SCAUSE (PT_SSTATUS + RV_XLEN)
#define PT_SIE (PT_SCAUSE + RV_XLEN)
#define PT_STVAL (PT_SIE + RV_XLEN)
#define PT_SEPC (PT_STVAL + RV_XLEN)
#define PT_HARTID (PT_SEPC + RV_XLEN)
#define PT_CTX_SIZE (PT_HARTID + RV_XLEN)

#ifndef __ASSEMBLER__

#include <attr.h>
#include <kern/lib/hart.h>
#include <list.h>
#include <stddef.h>
#include <types.h>

#define NCTX_MAX 512

struct context {
  union {
    struct {
      unsigned long regs[32];
    } __packed array;
    struct {
      unsigned long zero;
      unsigned long ra;
      unsigned long sp;
      unsigned long gp;
      unsigned long tp;
      unsigned long t0;
      unsigned long t1;
      unsigned long t2;
      unsigned long s0;
      unsigned long s1;
      unsigned long a0;
      unsigned long a1;
      unsigned long a2;
      unsigned long a3;
      unsigned long a4;
      unsigned long a5;
      unsigned long a6;
      unsigned long a7;
      unsigned long s2;
      unsigned long s3;
      unsigned long s4;
      unsigned long s5;
      unsigned long s6;
      unsigned long s7;
      unsigned long s8;
      unsigned long s9;
      unsigned long s10;
      unsigned long s11;
      unsigned long t3;
      unsigned long t4;
      unsigned long t5;
      unsigned long t6;
    } __packed names;
  } ctx_regs;
  unsigned long ctx_fregs[32];
  unsigned long ctx_sstatus;
  unsigned long ctx_scause;
  unsigned long ctx_sie;
  unsigned long ctx_stval;
  unsigned long ctx_sepc;
  unsigned long ctx_hartid;
  struct linked_node ctx_link;
};

struct trapframe {
  sys_addr_t tf_xstacktop;
  struct hlist_head tf_ctx_list;
};

extern struct context **cpus_context;

unsigned long intp_enable(void);
unsigned long intp_disable(void);
void intp_push(void);
void intp_pop(void);
void traps_init(void);
void trap_init_vec(void);

long ctx_alloc(struct context **ctx);
void ctx_free(struct context *ctx);

#endif
#endif

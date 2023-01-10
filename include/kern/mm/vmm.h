#ifndef _KERN_MM_VMM_H_
#define _KERN_MM_VMM_H_

#include <callback.h>
#include <stdint.h>

union sv39_perm_t {
  unsigned val : 10;
  struct {
    unsigned v : 1;
    unsigned r : 1;
    unsigned w : 1;
    unsigned x : 1;
    unsigned u : 1;
    unsigned g : 1;
    unsigned a : 1;
    unsigned d : 1;
    unsigned rsw : 2;
  } __attribute__((packed)) bits;
};

union sv39_va_t {
  uint64_t val;
  struct {
    unsigned off : 12;
    unsigned vpn0 : 9;
    unsigned vpn1 : 9;
    unsigned vpn2 : 9;
  } __attribute__((packed)) bits;
  struct {
    unsigned off : 12;
    unsigned vpn : 27;
  } __attribute__((packed)) pp;
};

union sv39_pa_t {
  uint64_t val;
  struct {
    unsigned off : 12;
    unsigned ppn0 : 9;
    unsigned ppn1 : 9;
    unsigned ppn2 : 26;
  } __attribute__((packed)) bits;
  struct {
    unsigned off : 12;
    unsigned long ppn : 44;
  } __attribute__((packed)) pp;
};

union sv39_entry_t {
  uint64_t val;
  struct {
    unsigned v : 1;
    unsigned r : 1;
    unsigned w : 1;
    unsigned x : 1;
    unsigned u : 1;
    unsigned g : 1;
    unsigned a : 1;
    unsigned d : 1;
    unsigned rsw : 2;
    unsigned ppn0 : 9;
    unsigned ppn1 : 9;
    unsigned ppn2 : 26;
    unsigned rsv : 10;
  } __attribute__((packed)) bits;
  struct {
    unsigned perm : 10;
    unsigned long ppn : 44;
    unsigned rsv : 10;
  } __attribute__((packed)) pp;
};

typedef union sv39_perm_t perm_t;
typedef union sv39_va_t vaddr_t;
typedef union sv39_pa_t paddr_t;
typedef union sv39_entry_t pte_t;

static inline paddr_t pte2pa(pte_t pte) {
  paddr_t pa = {.val = 0};
  pa.pp.ppn = pte.pp.ppn;
  return pa;
}

extern pte_t kern_pgdir[];

long pt_lookup(pte_t *pgdir, vaddr_t va, pte_t **res) __attribute__((warn_unused_result));
long pt_unmap(pte_t *pgdir, vaddr_t va) __attribute__((warn_unused_result));
long pt_map(pte_t *pgdir, vaddr_t va, paddr_t pa, perm_t perm)
    __attribute__((warn_unused_result));

void vmm_register_mmio(char *name, unsigned long *addr, unsigned long size);
void vmm_setup_mmio(void);
void vmm_setup_kern(void);
void vmm_start(void);
void vmm_summary(void);

#endif

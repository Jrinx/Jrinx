#ifndef _KERN_DRIVERS_DTB_H_
#define _KERN_DRIVERS_DTB_H_

#include <stdint.h>

struct fdt_header {
  uint32_t h_magic;
  uint32_t h_totalsize;
  uint32_t h_off_dt_struct;
  uint32_t h_off_dt_strings;
  uint32_t h_off_mem_rsvmap;
  uint32_t h_version;
  uint32_t h_last_comp_version;
  uint32_t h_boot_cpuid_phys;
  uint32_t h_size_dt_strings;
  uint32_t h_size_dt_struct;
};

#define DTB_FDT_MAGIC 0xd00dfeed
#define DTB_SUPPORTED_VER 17
#define DTB_COMPAT_VER 16

struct fdt_reserve_entry {
  uint64_t e_addr;
  uint64_t e_size;
};

#define FDT_BEGIN_NODE 0x00000001
#define FDT_END_NODE 0x00000002
#define FDT_PROP 0x00000003
#define FDT_NOP 0x00000004
#define FDT_END 0x00000009

#endif

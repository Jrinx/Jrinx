#ifndef _ELF_H_
#define _ELF_H_

#include <stdint.h>

#define ELF_CLASS_32 1
#define ELF_CLASS_64 2

#define ELF_DATA_EL 1
#define ELF_DATA_EB 2

#define ELF_CUR_VER 1

#define ET_NONE 0x00
#define ET_REL 0x01
#define ET_EXEC 0x02
#define ET_DYN 0x03
#define ET_CORE 0x04

#define EM_RISCV 0xf3

typedef unsigned long e_addr_t;
typedef unsigned long e_size_t;
typedef unsigned long e_offs_t;

struct elf_ehdr {
  union {
    uint8_t bytes[16];
    struct {
      uint8_t e_magic[4];
      uint8_t e_class;
      uint8_t e_data;
      uint8_t e_version;
      uint8_t e_osabi;
      uint8_t e_abiversion;
      uint8_t e_pad[7];
    } __attribute__((packed)) fields;
  } e_ident;
  uint16_t e_type;
  uint16_t e_machine;
  uint32_t e_version;
  e_addr_t e_entry;
  e_offs_t e_phoff;
  e_offs_t e_shoff;
  uint32_t e_flags;
  uint16_t e_ehsize;
  uint16_t e_phentsize;
  uint16_t e_phnum;
  uint16_t e_shentsize;
  uint16_t e_shnum;
  uint16_t e_shstrndx;
};

#define PT_NULL 0x00000000
#define PT_LOAD 0x00000001
#define PT_DYNAMIC 0x00000002
#define PT_INTERP 0x00000003
#define PT_NOTE 0x00000004
#define PT_SHLIB 0x00000005
#define PT_PHDR 0x00000006
#define PT_TLS 0x00000007
#define PT_LOOS 0x60000000
#define PT_HIOS 0x6fffffff
#define PT_LOPROC 0x70000000
#define PT_HIPROC 0x7fffffff

#define PF_X 0x1
#define PF_W 0x2
#define PF_R 0x4
#define PF_MASKPROC 0xf0000000

struct elf_phdr {
  uint32_t p_type;
#ifdef __LP64__
  uint32_t p_flags;
#endif
  e_offs_t p_offset;
  e_addr_t p_vaddr;
  e_addr_t p_paddr;
  e_size_t p_filesz;
  e_size_t p_memsz;
#ifndef __LP64__
  uint32_t p_flags;
#endif
  e_size_t p_align;
};

#define SHT_NULL 0x0
#define SHT_PROGBITS 0x1
#define SHT_SYMTAB 0x2
#define SHT_STRTAB 0x3
#define SHT_RELA 0x4
#define SHT_HASH 0x5
#define SHT_DYNAMIC 0x6
#define SHT_NOTE 0x7
#define SHT_NOBITS 0x8
#define SHT_REL 0x9
#define SHT_SHLIB 0x0a
#define SHT_DYNSYM 0x0b
#define SHT_INIT_ARRAY 0x0e
#define SHT_FINI_ARRAY 0x0f
#define SHT_PREINIT_ARRAY 0x10
#define SHT_GROUP 0x11
#define SHT_SYMTAB_SHNDX 0x12
#define SHT_NUM 0x13
#define SHT_LOOS 0x60000000

#define SHF_WRITE 0x1
#define SHF_ALLOC 0x2
#define SHF_EXECINSTR 0x4
#define SHF_MERGE 0x10
#define SHF_STRINGS 0x20
#define SHF_INFO_LINK 0x40
#define SHF_LINK_ORDER 0x80
#define SHF_OS_NONCONFORMING 0x100
#define SHF_GROUP 0x200
#define SHF_TLS 0x400
#define SHF_MASKOS 0x0ff00000
#define SHF_MASKPROC 0xf0000000
#define SHF_ORDERED 0x4000000
#define SHF_EXCLUDE 0x8000000

struct elf_shdr {
  uint32_t sh_name;
  uint32_t sh_type;
  e_size_t sh_flags;
  e_addr_t sh_addr;
  e_offs_t sh_offset;
  e_size_t sh_size;
  uint32_t sh_link;
  uint32_t sh_info;
  e_size_t sh_addralign;
  e_size_t sh_entsize;
};

#endif

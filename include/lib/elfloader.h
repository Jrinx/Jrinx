#ifndef _LIB_ELFLOADER_H_
#define _LIB_ELFLOADER_H_

#include <attr.h>
#include <callback.h>
#include <elf.h>
#include <stddef.h>

#define ELF_PHDR_ITER(ehdr, var)                                                               \
  for (Elf64_Phdr *var = (Elf64_Phdr *)((uint8_t *)(ehdr) + (ehdr)->e_phoff);                  \
       ((uint8_t *)(var) - ((uint8_t *)(ehdr) + (ehdr)->e_phoff)) / (ehdr)->e_phentsize <      \
       (ehdr)->e_phnum;                                                                        \
       var = (Elf64_Phdr *)((uint8_t *)(var) + (ehdr)->e_phentsize))

#define ELF_SHDR_ITER(ehdr, var)                                                               \
  for (Elf64_Shdr *var = (Elf64_Shdr *)((uint8_t *)(ehdr) + (ehdr)->e_shoff);                  \
       ((uint8_t *)(var) - ((uint8_t *)(ehdr) + (ehdr)->e_shoff)) / (ehdr)->e_shentsize <      \
       (ehdr)->e_shnum;                                                                        \
       var = (Elf64_Shdr *)((uint8_t *)(var) + (ehdr)->e_shentsize))

typedef long (*elf_mapper_t)(void *data, unsigned long va, size_t offset, const void *src,
                             size_t src_len);
typedef cb_typedef(elf_mapper_t) elf_mapper_callback_t;

const Elf64_Ehdr *elf64_from(const void *addr, size_t size);
long elf64_load_prog(Elf64_Phdr *phdr, const void *elf_addr,
                     elf_mapper_callback_t map_callback) __warn_unused_result;

#endif

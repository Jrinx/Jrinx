#ifndef _LIB_ELFLOADER_H_
#define _LIB_ELFLOADER_H_

#include <callback.h>
#include <elf.h>
#include <stddef.h>

#define ELF_PHDR_ITER(ehdr, var)                                                               \
  for (Elf64_Phdr *var = (Elf64_Phdr *)((uint8_t *)(ehdr) + (ehdr)->e_phoff);                  \
       ((uint8_t *)(var) - ((uint8_t *)(ehdr) + (ehdr)->e_phoff)) / (ehdr)->e_phentsize <      \
       (ehdr)->e_phnum;                                                                        \
       var = (Elf64_Phdr *)((uint8_t *)(var) + (ehdr)->e_phentsize))

typedef long (*elf_mapper_t)(void *data, unsigned long va, size_t offset, const void *src,
                             size_t src_len);
typedef cb_typedef(elf_mapper_t) elf_mapper_callback_t;

const Elf64_Ehdr *elf_from(const void *addr, size_t size);
long elf_load_prog(Elf64_Phdr *phdr, const void *elf_addr, elf_mapper_callback_t map_callback);

#endif

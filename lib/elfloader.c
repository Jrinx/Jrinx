#include <aligns.h>
#include <brpred.h>
#include <layouts.h>
#include <lib/elfloader.h>

const Elf64_Ehdr *elf_from(const void *addr, size_t size) {
  if (unlikely((unsigned long)addr % sizeof(Elf64_Ehdr) != 0)) {
    return NULL;
  }
  const Elf64_Ehdr *ehdr = addr;
  if (unlikely(ehdr->e_ident[EI_MAG0] != ELFMAG0 || ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
               ehdr->e_ident[EI_MAG2] != ELFMAG2 || ehdr->e_ident[EI_MAG3] != ELFMAG3)) {
    return NULL;
  }
  if (unlikely(ehdr->e_ident[EI_CLASS] != ELFCLASS64)) {
    return NULL;
  }
  if (unlikely(ehdr->e_ident[EI_DATA] != ELFDATA2LSB)) {
    return NULL;
  }
  if (unlikely(ehdr->e_ident[EI_VERSION] != EV_CURRENT)) {
    return NULL;
  }
  if (unlikely(ehdr->e_machine != EM_RISCV)) {
    return NULL;
  }
  return ehdr;
}

long elf_load_prog(Elf64_Phdr *phdr, const void *elf_addr, elf_mapper_callback_t map_callback) {
  unsigned long prog_addr = phdr->p_vaddr;
  size_t prog_size = phdr->p_filesz;
  size_t mem_size = phdr->p_memsz;

  long err;
  size_t src_len;
  size_t offset = prog_addr - align_down(prog_addr, PGSIZE);
  if (offset != 0) {
    src_len = prog_size < PGSIZE - offset ? prog_size : PGSIZE - offset;
    if ((err = cb_invoke(map_callback)(prog_addr, offset, elf_addr, src_len)) != 0) {
      return err;
    }
  }
  size_t i;
  for (i = offset != 0 ? src_len : 0; i < prog_size; i += PGSIZE) {
    src_len = prog_size - i < PGSIZE ? prog_size - i : PGSIZE;
    if ((err = cb_invoke(map_callback)(prog_addr + i, 0, elf_addr + i, src_len)) != 0) {
      return err;
    }
  }

  for (; i < mem_size; i += PGSIZE) {
    if ((err = cb_invoke(map_callback)(prog_addr + i, 0, NULL, 0)) != 0) {
      return err;
    }
  }
  return 0;
}

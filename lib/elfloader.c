#include <aligns.h>
#include <brpred.h>
#include <layouts.h>
#include <lib/elfloader.h>

const struct elf_ehdr *elf_from(const void *addr, size_t size) {
  if (unlikely((unsigned long)addr % sizeof(struct elf_ehdr) != 0)) {
    return NULL;
  }
  const struct elf_ehdr *ehdr = addr;
  if (unlikely(
          ehdr->e_ident.fields.e_magic[0] != 0x7f || ehdr->e_ident.fields.e_magic[1] != 'E' ||
          ehdr->e_ident.fields.e_magic[2] != 'L' || ehdr->e_ident.fields.e_magic[3] != 'F')) {
    return NULL;
  }
  if (unlikely(ehdr->e_ident.fields.e_class != ELF_CLASS_64)) {
    return NULL;
  }
  if (unlikely(ehdr->e_ident.fields.e_data != ELF_DATA_EL)) {
    return NULL;
  }
  if (unlikely(ehdr->e_ident.fields.e_version != ELF_CUR_VER)) {
    return NULL;
  }
  if (unlikely(ehdr->e_machine != EM_RISCV)) {
    return NULL;
  }
  return ehdr;
}

long elf_load_prog(struct elf_phdr *phdr, const void *elf_addr,
                   elf_mapper_callback_t map_callback) {
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
    src_len = prog_size - i < PGSIZE ? prog_size - i : PGSIZE;
    if ((err = cb_invoke(map_callback)(prog_addr + i, 0, NULL, src_len)) != 0) {
      return err;
    }
  }
  return 0;
}

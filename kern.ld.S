#include <layouts.h>

ENTRY(_start)

SECTIONS {
  . = KERNBASE;
  . = ALIGN(PGSIZE);
  .text : {
    *(.boot)
    *(.text .text.*)
  }
  PROVIDE(kern_text_end = .);
  . = ALIGN(PGSIZE);
  .rodata : {
    *(.rodata .rodata.*)
  }

  . = ALIGN(PGSIZE);
  .data : {
    *(.data .data.*)
  }

  . = ALIGN(PGSIZE);
  .bss : {
    *(.bss .bss.* .sbss .sbss.*)
  }

  . = ALIGN(PGSIZE);
  .ksec : {
    *(.ksec .ksec.*)
  }

  PROVIDE(kern_end = .);
}

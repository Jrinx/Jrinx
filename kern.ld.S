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
    *(.ksec.stack)
    PROVIDE(kern_dev_init_begin = .);
    *(.ksec.dev_init_highest.*)
    *(.ksec.dev_init_high.*)
    *(.ksec.dev_init_medium.*)
    *(.ksec.dev_init_low.*)
    *(.ksec.dev_init_lowest.*)
    PROVIDE(kern_dev_init_end = .);
  }

  PROVIDE(kern_end = .);
}

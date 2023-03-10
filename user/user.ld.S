#include <layouts.h>

ENTRY(_start)

SECTIONS {
  . = USERBASE;
  . = ALIGN(PGSIZE);
  .text : {
    *(.text .text.*)
  }
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
}

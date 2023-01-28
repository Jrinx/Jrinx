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
    PROVIDE(kern_dev_init_begin = .);
    *(.ksec.dev_init_highest.*)
    *(.ksec.dev_init_high.*)
    *(.ksec.dev_init_medium.*)
    *(.ksec.dev_init_low.*)
    *(.ksec.dev_init_lowest.*)
    PROVIDE(kern_dev_init_end = .);
    PROVIDE(kern_lock_impl_begin = .);
    *(.ksec.lock_impl.*)
    PROVIDE(kern_lock_impl_end = .);
    PROVIDE(kern_testset_begin = .);
    *(.ksec.testcase.*)
    PROVIDE(kern_testset_end = .);
  }

  . = ALIGN(PGSIZE);
  .bss : {
    *(.bss .bss.* .sbss .sbss.*)
  }

  . = ALIGN(PGSIZE);
  .kern_master_stack : {
    *(.ksec.master_stack)
  }

  PROVIDE(kern_end = .);
}

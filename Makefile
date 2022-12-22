CROSS_COMPILE	:= riscv64-unknown-elf-

__CC		:= $(CROSS_COMPILE)gcc
__CPP		:= $(CROSS_COMPILE)cpp
__LD		:= $(CROSS_COMPILE)ld
OBJDUMP		:= $(CROSS_COMPILE)objdump
OBJCOPY		:= $(CROSS_COMPILE)objcopy
GDB		:= gdb-multiarch

include mk/conf.mk

CFLAGS		+= --std=gnu99 -nostdlib \
		-Wall -Werror -Wa,--fatal-warnings \
		-mabi=lp64 -march=rv64g -mcmodel=medany -mno-relax \
		-fno-omit-frame-pointer -ffreestanding -fno-common -fno-stack-protector -fno-builtin

LDFLAGS		+= --fatal-warnings --warn-unresolved-symbols

EMU 		:= qemu-system-riscv64
EMU_MACH 	:= sifive_u
EMU_CPUS 	:= 5
EMU_RAM_SIZE	:= 1G
EMU_OPTS	:= -M $(EMU_MACH) -m $(EMU_RAM_SIZE) -nographic -smp $(EMU_CPUS)

INCLUDES	:= -I./include
MODULES		:= init kern lib
OBJECTS		:= $(addsuffix /**/*.o, $(MODULES))
LDSCRIPT	:= kern.ld
TARGET_DIR	:= target
JRINX		:= $(TARGET_DIR)/jrinx

CFLAGS		+= -DCONFIG_NR_CORES=$(EMU_CPUS)
EMU_OPTS	+= -kernel $(JRINX)

export __CC __CPP __LD OBJDUMP OBJCOPY CFLAGS LDFLAGS

.ONESHELL:
.PHONY: all clean objdump run gdb dbg $(JRINX) $(MODULES)

all:
	@export MAKEFLAGS="-j$$(nproc) -s $$MAKEFLAGS"
	@$(MAKE) $(JRINX)

$(JRINX): SHELL := /bin/bash
$(JRINX): $(MODULES) $(LDSCRIPT)
	mkdir -p $(TARGET_DIR)
	shopt -s nullglob globstar
	$(LD) $(LDFLAGS) -T $(LDSCRIPT) -o $(JRINX) $(OBJECTS)

$(MODULES):
	$(MAKE) -C $@

include mk/compile.mk

clean:
	@rm -rf $(TARGET_DIR)
	@find -- . \( -name '*.o' -o -name '*.ld' \) -delete

objdump:
	@$(OBJDUMP) -aldS $(JRINX) > $(JRINX).objdump

objcopy:
	@$(OBJCOPY) -O binary $(JRINX) $(JRINX).bin

run:
	@$(EMU) $(EMU_OPTS)

dbg: EMU_OPTS	+= -s -S
dbg: CFLAGS	+= -DJRINX=$(JRINX)
dbg: | .gdbinit run
	@$(RM) .gdbinit

check-style:
	@scripts/check-style

fix-style:
	@scripts/check-style -f

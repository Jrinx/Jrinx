OPENSBI_FW_PATH	?= ../archive/opensbi/build/platform/generic/firmware

# Qemu dose not support big endian now.
TARGET_ENDIAN	?= little

# Used to check macro expansion.
CHECK_PREPROC	?= n

CROSS_COMPILE	:= riscv64-unknown-elf-

__CC		:= $(CROSS_COMPILE)gcc
__CPP		:= $(CROSS_COMPILE)cpp
__LD		:= $(CROSS_COMPILE)ld
OBJDUMP		:= $(CROSS_COMPILE)objdump
OBJCOPY		:= $(CROSS_COMPILE)objcopy
GDB		:= gdb-multiarch
GDB_EVAL_CMD	:= -ex 'target remote :1234'

include mk/conf.mk

CFLAGS		+= --std=gnu99 -nostdlib \
		-Wall -Werror -Wa,--fatal-warnings \
		-mabi=lp64 -march=rv64g -m$(TARGET_ENDIAN)-endian -mcmodel=medany -mno-relax \
		-fno-omit-frame-pointer -ffreestanding -fno-common -fno-stack-protector -fno-builtin

ifeq ($(TARGET_ENDIAN),little)
	CFLAGS	+= -DCONFIG_LITTLE_ENDIAN=1
else
	CFLAGS	+= -DCONFIG_LITTLE_ENDIAN=0
endif

LDFLAGS		+= --fatal-warnings --warn-unresolved-symbols

EMU 		:= qemu-system-riscv64
EMU_MACH 	:= virt
EMU_CPUS 	:= 5
EMU_RAM_SIZE	:= 1G
EMU_OPTS	:= -M $(EMU_MACH) -m $(EMU_RAM_SIZE) -nographic -smp $(EMU_CPUS)

INCLUDES	:= -I./include
MODULES		:= kern lib
OBJECTS		:= $(addsuffix /**/*.o, $(MODULES))
LDSCRIPT	:= kern.ld
TARGET_DIR	:= target
JRINX		:= $(TARGET_DIR)/jrinx
BOOTLOADER	:= $(OPENSBI_FW_PATH)/fw_jump.elf

DTC		:= dtc

export __CC __CPP __LD OBJDUMP OBJCOPY CFLAGS LDFLAGS CHECK_PREPROC

.ONESHELL:
.PHONY: all clean objdump objcopy run dbg gdb gdb-sbi dumpdtb dumpdts $(JRINX) $(MODULES)

all:
	@export MAKEFLAGS="-j$$(nproc) -s $$MAKEFLAGS"
	@$(MAKE) $(JRINX)

$(JRINX): SHELL := /bin/bash
$(JRINX): $(MODULES) $(LDSCRIPT) $(TARGET_DIR)
	shopt -s nullglob globstar
	$(LD) $(LDFLAGS) -T $(LDSCRIPT) -o $(JRINX) $(OBJECTS)

$(MODULES):
	$(MAKE) -C $@

$(TARGET_DIR):
	mkdir -p $(TARGET_DIR)

include mk/compile.mk

clean:
	@rm -rf $(TARGET_DIR)
	@find -- . \( \
		-name '*.o' -o -name '*.ld' -o -name '*.dtb' -o -name '*.dts' -o \
		-name '*.i' \
	\) -delete

objdump:
	@find -- * \( -path $(JRINX) \) -exec \
		sh -c '$(OBJDUMP) {} -aldS > {}.objdump && echo {}.objdump' ';'

objcopy:
	@$(OBJCOPY) -O binary $(JRINX) $(JRINX).bin

run: EMU_OPTS		+= -kernel $(JRINX) -bios $(BOOTLOADER)
run:
	@$(EMU) $(EMU_OPTS)

dbg: EMU_OPTS		+= -s -S
dbg: CFLAGS		+= -DJRINX=$(JRINX)
dbg: run

gdb:
	@$(GDB) $(GDB_EVAL_CMD) $(JRINX)

gdb-sbi: GDB_EVAL_CMD	+= -ex 'set confirm off' -ex 'add-symbol-file $(BOOTLOADER)' \
			-ex 'set confirm on'
gdb-sbi: gdb

dumpdtb: EMU_OPTS	+= -M $(EMU_MACH),dumpdtb=$(EMU_MACH).dtb
dumpdtb: run

dumpdts: dumpdtb
	@$(DTC) -I dtb -O dts $(EMU_MACH).dtb -o $(EMU_MACH).dts

check-style:
	@scripts/check-style

fix-style:
	@scripts/check-style -f

register-git-hooks:
	@ln -s ../../scripts/pre-commit .git/hooks/pre-commit

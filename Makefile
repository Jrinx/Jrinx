OPENSBI_FW_PATH	?= ../archive/opensbi/build/platform/generic/firmware

# Qemu does not support big endian now.
TARGET_ENDIAN	?= little

CPUS		?= 5

COLOR		?= y

JRINX_LOGO	:= jrinx.logo
CROSS_COMPILE	:= riscv64-unknown-elf-

GDB		:= gdb-multiarch
GDB_EVAL_CMD	:= -ex 'target remote :1234'

CFLAGS		+= --std=gnu99 -nostdlib \
		-Wall -Werror -Wa,--fatal-warnings \
		-mabi=lp64 -march=rv64g -m$(TARGET_ENDIAN)-endian -mcmodel=medany -mno-relax \
		-fno-omit-frame-pointer -ffreestanding -fno-common -fno-stack-protector -fno-builtin \
		-DCONFIG_ENDIAN=$(shell echo $(TARGET_ENDIAN) | tr '[:lower:]' '[:upper:]')_ENDIAN \
		-DCONFIG_COLOR=$(shell [ "$(COLOR)" = "y" ] && echo 1 || echo 0) \
		-DCONFIG_JRINX_LOGO=$(shell scripts/logo-gen $(JRINX_LOGO))

LDFLAGS		+= --fatal-warnings --warn-unresolved-symbols

EMU 		:= qemu-system-riscv64
EMU_MACH 	:= virt
EMU_CPUS 	:= $(CPUS)
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

export CROSS_COMPILE CFLAGS LDFLAGS
export CHECK_PREPROC ?= n

.ONESHELL:
.PHONY: all debug release release-debug build clean clean-all run dbg gdb gdb-sbi \
	preprocess objdump objcopy dumpdtb dumpdts \
	$(JRINX) $(MODULES) \
	check-style fix-style register-git-hooks

all: debug

release: CFLAGS		+= -O2
release: LDFLAGS	+= -O --gc-sections
release: build

release-debug: CFLAGS	+= -g -ggdb
release-debug: release

debug: CFLAGS		+= -O0 -g -ggdb
debug: build

ifneq ($(filter preprocess,$(MAKECMDGOALS)),)
build: CHECK_PREPROC	:= y
endif
build: clean
	@export MAKEFLAGS="-j$$(nproc) -s $$MAKEFLAGS"
	@$(MAKE) $(JRINX)

$(JRINX): SHELL := $(shell which bash)
$(JRINX): $(MODULES) $(LDSCRIPT) $(TARGET_DIR)
	shopt -s nullglob globstar
	$(LD) $(LDFLAGS) -T $(LDSCRIPT) -o $(JRINX) $(OBJECTS)

$(MODULES):
	$(MAKE) -C $@

$(TARGET_DIR):
	@mkdir -p $@

include mk/compile.mk

clean:
	@rm -rf $(TARGET_DIR)
	@find -- . \( \
		-name '*.o' -o -name '*.ld' -o -name '*.i' \
	\) -type f -delete

clean-all: clean
	@find -- . \( \
		-name '*.dtb' -o -name '*.dts' \
	\) -type f -delete

preprocess: all

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
dumpdtb:
	@$(EMU) $(EMU_OPTS)

dumpdts: dumpdtb
	@$(DTC) -I dtb -O dts $(EMU_MACH).dtb -o $(EMU_MACH).dts

check-style: clean-all
	@scripts/check-style

fix-style: clean-all
	@scripts/check-style -f

register-git-hooks:
	@ln -s ../../scripts/pre-commit .git/hooks/pre-commit

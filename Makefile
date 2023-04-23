# Qemu does not support big endian now.
TARGET_ENDIAN	:= little

CPUS		?= 5
COLOR		?= y
DTB		?=
ARGS		?=
SYSCONF		?=
BOARD		?= virt
CROSS_COMPILE	?= riscv64-unknown-linux-gnu-

JRINX_LOGO	:= jrinx.logo

GDB		:= gdb-multiarch
GDB_EVAL_CMD	:= -ex 'target remote :1234'

CFLAGS		+= --std=gnu99 -nostdlib \
		-Wall -Werror -Wa,--fatal-warnings \
		-mabi=lp64 -march=rv64g -m$(TARGET_ENDIAN)-endian -mcmodel=medany -mno-relax \
		-fno-omit-frame-pointer -ffreestanding -fno-common -fno-stack-protector -fno-builtin \
		-DCONFIG_ENDIAN=$(shell echo $(TARGET_ENDIAN) | tr '[:lower:]' '[:upper:]')_ENDIAN \
		-DCONFIG_COLOR=$(shell [ "$(COLOR)" = "y" ] && echo 1 || echo 0) \
		-DCONFIG_JRINX_LOGO='$(shell scripts/logo-gen $(JRINX_LOGO))' \
		-DCONFIG_REVISON='"$(shell git rev-parse --short HEAD)"'

LDFLAGS		+= --fatal-warnings --warn-unresolved-symbols

EMU 		:= qemu-system-riscv64
EMU_MACH 	:= $(BOARD)
EMU_CPUS 	:= $(CPUS)
EMU_RAM_SIZE	:= 1G
EMU_ARGS	:= $(ARGS) $(shell if [ -n "$(SYSCONF)" ]; then ./scripts/sysconf $(SYSCONF); fi)
EMU_OPTS	:= -M $(EMU_MACH) -m $(EMU_RAM_SIZE) -nographic -smp $(EMU_CPUS) -no-reboot
ifneq ($(DTB),)
EMU_OPTS	+= -dtb $(DTB)
endif

MODULES		:= kern
USER_MODULES	:= user
LIB_MODULES	:= lib
OBJECTS		:= $(addsuffix /**/*.o, $(MODULES) $(LIB_MODULES)) \
		$(addsuffix /**/*.x, $(USER_MODULES))
LDSCRIPT	:= kern.ld
TARGET_DIR	:= target
JRINX		:= $(TARGET_DIR)/jrinx

OPENSBI_ROOT	:= opensbi
OPENSBI_FW_PATH	:= $(OPENSBI_ROOT)/build/platform/generic/firmware
OPENSBI_FW_JUMP	:= $(OPENSBI_FW_PATH)/fw_jump.elf

DTC		:= dtc

export CROSS_COMPILE CFLAGS LDFLAGS
export LIB_MODULES
export CHECK_PREPROC ?= n
export BUILD_ROOT_DIR ?= $(abspath ./)

MAKEFLAGS	:= -j$(shell nproc) -s $(MAKEFLAGS)

.ONESHELL:
.PHONY: all debug release release-debug build sbi-fw clean clean-dt clean-opensbi clean-all \
	run dbg gdb gdb-sbi \
	preprocess objdump objcopy mkimage dumpdts \
	$(JRINX) $(MODULES) $(USER_MODULES) $(LIB_MODULES) \
	check-style fix-style register-git-hooks cloc

all: debug

release: CFLAGS		+= -O2
release: LDFLAGS	+= -O --gc-sections
release: build

release-debug: CFLAGS	+= -g -ggdb
release-debug: release

debug: CFLAGS		+= -O0 -g -ggdb
debug: build

build: | clean $(JRINX)

$(JRINX): SHELL := $(shell which bash)
$(JRINX): $(MODULES) $(USER_MODULES) $(LDSCRIPT) $(TARGET_DIR)
	shopt -s nullglob globstar
	$(LD) $(LDFLAGS) -T $(LDSCRIPT) -o $(JRINX) $(OBJECTS)

$(MODULES): $(LIB_MODULES)
$(USER_MODULES): $(LIB_MODULES)

$(MODULES) $(USER_MODULES) $(LIB_MODULES):
	$(MAKE) -C $@ $(shell \
	if [ ! -f $@/Makefile ]; then \
		echo '-f $(BUILD_ROOT_DIR)/mk/auto-make.mk'; \
	fi)

$(OPENSBI_FW_JUMP): sbi-fw

sbi-fw:
	@$(MAKE) -C $(OPENSBI_ROOT) all PLATFORM=generic PLATFORM_RISCV_XLEN=64

$(TARGET_DIR):
	@mkdir -p $@

include mk/compile.mk

clean:
	@rm -rf $(TARGET_DIR)
	@find -- . -not \( -path './$(OPENSBI_ROOT)/*' \) \( \
		-name '*.o' -o -name '*.b' -o -name '*.b.c' -o -name '*.x' -o \
		-name '*.ld' -o -name '*.i' -o -name '$(EMU_TEST_CONF)' -o -name '*.objdump' \
	\) -type f -delete

clean-dt:
	@find -- . -not \( -path './$(OPENSBI_ROOT)/*' \) \( \
		-name '*.dtb' -o -name '*.dts' \
	\) -type f -delete

clean-opensbi:
	@$(MAKE) -C $(OPENSBI_ROOT) distclean

clean-all: clean clean-dt clean-opensbi

preprocess: CHECK_PREPROC := y
preprocess: all

objdump:
	@find -- * \( -path $(JRINX) -o -name '*.b' \) -exec \
		sh -c '$(OBJDUMP) {} -aldS > {}.objdump && echo {}.objdump' ';'

objcopy:
	@$(OBJCOPY) -O binary $(JRINX) $(JRINX).bin

mkimage: objcopy
	mkimage -A riscv -O linux -C none -a 0x80200000 -e 0x80200000 \
		-n Jrinx -d $(JRINX).bin $(JRINX).uImage

run: EMU_OPTS			+= -kernel $(JRINX) -bios $(OPENSBI_FW_JUMP) -append '$(EMU_ARGS)'
run: $(OPENSBI_FW_JUMP)
	@$(EMU) $(EMU_OPTS)

dbg: EMU_OPTS			+= -s -S
dbg: run

gdb:
	@$(GDB) $(GDB_EVAL_CMD) $(JRINX)

gdb-sbi: GDB_EVAL_CMD		+= -ex 'set confirm off' -ex 'add-symbol-file $(OPENSBI_FW_JUMP)' \
				-ex 'set confirm on'
gdb-sbi: gdb

dumpdts: $(EMU_MACH).dts

%.dts: %.dtb
	@$(DTC) -I dtb -O dts $< -o $@

$(EMU_MACH).dtb: EMU_OPTS	+= -M $(EMU_MACH),dumpdtb=$(EMU_MACH).dtb
$(EMU_MACH).dtb:
	@$(EMU) $(EMU_OPTS)

check-style:
	@scripts/check-style

fix-style:
	@scripts/check-style -f

register-git-hooks:
	@ln -s ../../scripts/pre-commit .git/hooks/pre-commit

cloc:
	@cloc --exclude-dir=$(OPENSBI_ROOT) .

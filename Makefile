CMD_PREFIX		?= @
CPUS			?= 5
COLOR			?= y
DTB			?=
ARGS			?=
SYSCONF			?=
MINTICK			?= 10
BOARD			?= virt
BUILD_3RD_PARTY		?= n

ENDIAN			=  little
BUILD_ROOT_DIR		=  $(CURDIR)
JRINX_LOGO		=  $(BUILD_ROOT_DIR)/jrinx.logo

CROSS_COMPILE		?= riscv64-unknown-linux-gnu-
CC			=  $(CROSS_COMPILE)gcc
OBJCOPY			=  $(CROSS_COMPILE)objcopy
OBJDUMP			=  $(CROSS_COMPILE)objdump
STRIP			=  $(CROSS_COMPILE)strip
CPP			=  $(CC) -E
LDSPP			=  $(CC) -E
AS			=  $(CC)
LD			=  $(CC)
DTC			=  dtc
LOGOGEN			=  $(BUILD_ROOT_DIR)/scripts/logo-gen
BINTOC			=  $(BUILD_ROOT_DIR)/scripts/bintoc
SYSCONFTR		=  $(BUILD_ROOT_DIR)/scripts/sysconf
CHECKSTYLE		=  $(BUILD_ROOT_DIR)/scripts/check-style
FIXSTYLE		=  $(CHECKSTYLE) --fix
CLOC			=  cloc --vcs=git

ifeq ($(findstring release,$(MAKECMDGOALS)),)
MODE			=  debug
else
MODE			=  release
endif

PREV_MODE		=  $(shell cat $(target-dir)/.compile-mode 2>/dev/null)

GENFLAGS		+= -I$(BUILD_ROOT_DIR)/include
GENFLAGS		+= -DCONFIG_ENDIAN=$(shell echo $(ENDIAN) | tr '[:lower:]' '[:upper:]')_ENDIAN
GENFLAGS		+= -DCONFIG_COLOR=$(shell [ "$(COLOR)" = "y" ] && echo 1 || echo 0)
GENFLAGS		+= -DCONFIG_MINTICK=$(MINTICK)
GENFLAGS		+= -DCONFIG_JRINX_LOGO='$(shell $(LOGOGEN) $(JRINX_LOGO))'
GENFLAGS		+= -DCONFIG_REVISION='"$(shell git rev-parse --short HEAD)"'

CFLAGS			=  $(GENFLAGS)
CFLAGS			+= --std=gnu99 -g
CFLAGS			+= -nostdlib
CFLAGS			+= -Wall -Werror
CFLAGS			+= -mabi=lp64d -march=rv64imafd -m$(ENDIAN)-endian -mcmodel=medany -mno-relax
CFLAGS			+= -ffreestanding
CFLAGS			+= -fno-common -fno-stack-protector -fno-builtin -fno-omit-frame-pointer

ifeq ($(MODE),release)
CFLAGS			+= -O2
else
CFLAGS			+= -O0 -ggdb
endif

CPPFLAGS		=  $(GENFLAGS)
CPPFLAGS		+= -P

LDSPPFLAGS		=  $(GENFLAGS)
LDSPPFLAGS		+= -P -x c

ASFLAGS			=  $(CFLAGS)
ASFLAGS			+= -Wa,-mno-relax,--fatal-warnings

LDFLAGS			=  $(CFLAGS)
LDFLAGS			+= -static
LDFLAGS			+= -Wl,--fatal-warnings,--warn-unresolved-symbols,--build-id=none

ifeq ($(MODE),release)
LDFLAGS			+= -Wl,-O,--gc-sections
endif

source-dir		:= $(BUILD_ROOT_DIR)
target-dir		:= target
kern-mods		:= kern
user-lib-mods		:= user/crt user/lib
user-exe-mods		:= user/app user/test
user-3rd-mods		:= user/third-party
lib-mods		:= lib

define probe-objs
	$(shell find $(1) \( -name '*.c' -a -not -path '$(user-3rd-mods)/*' \) | \
		sed 's/\.c/\.$(2)/g') \
	$(shell find $(1) \( -name '*.S' -a -not -path '$(user-3rd-mods)/*' \) | \
		sed 's/\.S/\.$(2)/g')
endef

kern-lds		=  kern.ld
kern-objs		=  $(call probe-objs,$(kern-mods),o)
lib-objs		=  $(call probe-objs,$(lib-mods),o)
user-lds		=  user/user.ld
user-lib-objs		=  $(call probe-objs,$(user-lib-mods),o)
user-exe-objs		=  $(call probe-objs,$(user-exe-mods),x)

kern-lds-path		=  $(addprefix $(target-dir)/,$(kern-lds))
kern-objs-path		=  $(addprefix $(target-dir)/,$(kern-objs))
lib-objs-path		=  $(addprefix $(target-dir)/,$(lib-objs))
user-lds-path		=  $(addprefix $(target-dir)/,$(user-lds))
user-lib-objs-path	=  $(addprefix $(target-dir)/,$(user-lib-objs))
user-exe-objs-path	=  $(addprefix $(target-dir)/,$(user-exe-objs))

deps			=  $(kern-lds-path:.ld=.dep)
deps			+= $(kern-objs-path:.o=.dep)
deps			+= $(lib-objs-path:.o=.dep)
deps			+= $(user-lds-path:.ld=.dep)
deps			+= $(user-lib-objs-path:.o=.dep)
deps			+= $(user-exe-objs-path:.x=.dep)

preproc			=  $(kern-objs-path:.o=.i)
preproc			+= $(lib-objs-path:.o=.i)
preproc			+= $(user-lib-objs-path:.o=.i)

targets			=  $(target-dir)/jrinx $(target-dir)/.compile-mode
ifneq ($(findstring preprocess,$(MAKECMDGOALS)),)
targets			+= $(preproc)
endif

COMPILE_CC_DEP		= \
			$(CMD_PREFIX)mkdir -p $$(dirname $(1)); \
			echo " CC-DEP    $(subst $(target-dir)/,,$(1))"; \
			printf %s $$(dirname $(1))/ > $(1) && \
			$(CC) $(CFLAGS) -MM $(2) >> $(1) || \
			rm -f $(1)
COMPILE_CC		= \
			$(CMD_PREFIX)mkdir -p $$(dirname $(1)); \
			echo " CC        $(subst $(target-dir)/,,$(1))"; \
			$(CC) $(CFLAGS) -c $(2) -o $(1)

COMPILE_AS_DEP		= \
			$(CMD_PREFIX)mkdir -p $$(dirname $(1)); \
			echo " AS-DEP    $(subst $(target-dir)/,,$(1))"; \
			printf %s $$(dirname $(1))/ > $(1) && \
			$(CC) $(ASFLAGS) -MM $(2) >> $(1) || \
			rm -f $(1)
COMPILE_AS		= \
			$(CMD_PREFIX)mkdir -p $$(dirname $(1)); \
			echo " AS        $(subst $(target-dir)/,,$(1))"; \
			$(AS) $(ASFLAGS) -c $(2) -o $(1)

COMPILE_CPP_DEP		= \
			$(CMD_PREFIX)mkdir -p $$(dirname $(1)); \
			echo " CPP-DEP   $(subst $(target-dir)/,,$(1))"; \
			printf %s $$(dirname $(1))/ > $(1) && \
			$(CC) $(CPPFLAGS) -MM $(3) -MT $$(basename $(1:.dep=$(2))) >> $(1) || \
			rm -f $(1)
COMPILE_CPP		= \
			$(CMD_PREFIX)mkdir -p $$(dirname $(1)); \
			echo " CPP       $(subst $(target-dir)/,,$(1))"; \
			$(CPP) $(CPPFLAGS) $(2) > $(1)

COMPILE_LDSPP_DEP	= \
			$(CMD_PREFIX)mkdir -p $$(dirname $(1)); \
			echo " LDSPP-DEP $(subst $(target-dir)/,,$(1))"; \
			printf %s $$(dirname $(1))/ > $(1) && \
			$(CC) $(LDSPPFLAGS) -MM $(3) -MT $$(basename $(1:.dep=$(2))) >> $(1) || \
			rm -f $(1)

COMPILE_LDSPP		= \
			$(CMD_PREFIX)mkdir -p $$(dirname $(1)); \
			echo " LDSPP     $(subst $(target-dir)/,,$(1))"; \
			$(LDSPP) $(LDSPPFLAGS) $(2) > $(1)

COMPILE_LINK		= \
			$(CMD_PREFIX)mkdir -p $$(dirname $(1)); \
			echo " LINK      $(subst $(target-dir)/,,$(1))"; \
			$(LD) $(LDFLAGS) $(3) -Wl,-T$(2) -o $(1)

COMPILE_BINTOC		= \
			$(CMD_PREFIX)mkdir -p $$(dirname $(1)); \
			echo " BINTOC    $(subst $(target-dir)/,,$(1))"; \
			$(BINTOC) $(2) -p $(3) -o $(1)

MAKEFLAGS		:= -j$(shell nproc) -s $(MAKEFLAGS) -r
OPENSBI_MAKEFLAGS	:= $(MAKEFLAGS)
ifneq ($(MODE),$(PREV_MODE))
MAKEFLAGS		+= -B
endif

.PHONY: all
all: $(targets)

.PHONY: debug release preprocess
debug release preprocess: all

.PHONY: third-party
third-party:
	$(CMD_PREFIX)BUILD_ROOT_DIR=$(BUILD_ROOT_DIR) $(MAKE) -C $(user-3rd-mods)

.SECONDARY:

.ONESHELL:
ifeq ($(BUILD_3RD_PARTY),y)
$(target-dir)/jrinx: third-party
endif
$(target-dir)/jrinx: $(kern-lds-path) $(kern-objs-path) $(lib-objs-path) $(user-exe-objs-path)
ifeq ($(BUILD_3RD_PARTY),y)
	$(call COMPILE_LINK,$@,$(kern-lds-path), \
		$(kern-objs-path) $(lib-objs-path) $(user-exe-objs-path) $(wildcard $(user-3rd-mods)/*.x))
else
	$(call COMPILE_LINK,$@,$(kern-lds-path), \
		$(kern-objs-path) $(lib-objs-path) $(user-exe-objs-path))
endif

$(target-dir)/.compile-mode:
	$(CMD_PREFIX)echo '$(MODE)' > $@

$(target-dir)/%.i: $(source-dir)/%.c
	$(call COMPILE_CPP,$@,$<)

$(target-dir)/%.i: $(source-dir)/%.S
	$(call COMPILE_CPP,$@,$<)

$(target-dir)/%.dep: $(source-dir)/%.c
	$(call COMPILE_CC_DEP,$@,$<)

$(target-dir)/%.o: $(source-dir)/%.c
	$(call COMPILE_CC,$@,$<)

$(target-dir)/%.dep: $(source-dir)/%.S
	$(call COMPILE_AS_DEP,$@,$<)

$(target-dir)/%.o: $(source-dir)/%.S
	$(call COMPILE_AS,$@,$<)

$(target-dir)/%.dep: $(source-dir)/%.ldS
	$(call COMPILE_LDSPP_DEP,$@,.ld,$<)

$(target-dir)/%.ld: $(source-dir)/%.ldS
	$(call COMPILE_LDSPP,$@,$<)

$(target-dir)/%.x: $(target-dir)/%.b.c
	$(call COMPILE_CC,$@,$<)

$(target-dir)/%.b.c: $(target-dir)/%.b
	$(call COMPILE_BINTOC,$@,$<,$(lastword $(subst /, ,$(dir $(abspath $@)))))

$(target-dir)/%.b: $(target-dir)/%.o $(user-lds-path) $(lib-objs-path) $(user-lib-objs-path)
	$(call COMPILE_LINK,$@,$(user-lds-path), \
		$< $(lib-objs-path) $(user-lib-objs-path))

ifeq ($(findstring clean,$(MAKECMDGOALS)),)
-include $(deps)
endif

.PHONY: objdump objcopy strip mkimage
objdump:
	$(CMD_PREFIX)find $(target-dir) \( -path $(target-dir)/jrinx -o -name '*.b' \) -exec \
		sh -c '$(OBJDUMP) {} -aldS > {}.objdump && echo " OBJDUMP > {}.objdump"' \;

objcopy:
	$(CMD_PREFIX)$(OBJCOPY) -O binary $(target-dir)/jrinx $(target-dir)/jrinx.bin

strip:
	$(CMD_PREFIX)$(STRIP) $(target-dir)/jrinx

mkimage: | strip objcopy
	$(CMD_PREFIX)mkimage -A riscv -O linux -C none -a 0x80200000 -e 0x80200000 \
		-n Jrinx -d $(target-dir)/jrinx.bin $(target-dir)/jrinx.uImage

.PHONY: clean distclean 3rdclean
clean:
	$(CMD_PREFIX)mkdir -p $(target-dir)
	$(CMD_PREFIX)find $(target-dir) -type f -name '*.i' -delete
	$(CMD_PREFIX)find $(target-dir) -type f -name '*.o' -delete
	$(CMD_PREFIX)find $(target-dir) -type f -name '*.b' -delete
	$(CMD_PREFIX)find $(target-dir) -type f -name '*.b.c' -delete
	$(CMD_PREFIX)find $(target-dir) -type f -name '*.x' -delete
	$(CMD_PREFIX)find $(target-dir) -type f -name '*.ld' -delete

distclean:
	$(CMD_PREFIX)rm -rf $(target-dir)

3rdclean:
	$(CMD_PREFIX)$(MAKE) -C $(user-3rd-mods) clean

OPENSBI_ROOT		:= $(BUILD_ROOT_DIR)/opensbi
OPENSBI_FW_PATH		:= $(OPENSBI_ROOT)/build/platform/generic/firmware
OPENSBI_FW_JUMP		:= $(OPENSBI_FW_PATH)/fw_jump.elf

.PHONY: opensbi clean-opensbi distclean-opensbi
opensbi: export MAKEFLAGS=$(OPENSBI_MAKEFLAGS)
opensbi:
	$(CMD_PREFIX)$(MAKE) -C $(OPENSBI_ROOT) all \
		CROSS_COMPILE=$(CROSS_COMPILE) \
		PLATFORM=generic \
		PLATFORM_RISCV_XLEN=64

clean-opensbi:
	$(CMD_PREFIX)$(MAKE) -C $(OPENSBI_ROOT) clean

distclean-opensbi:
	$(CMD_PREFIX)$(MAKE) -C $(OPENSBI_ROOT) distclean

EMU			:= qemu-system-riscv64
EMU_RAM_SIZE		:= 1G
EMU_ARGS		:= $(ARGS) $(shell if [ -n "$(SYSCONF)" ]; then $(SYSCONFTR) $(SYSCONF); fi)
EMU_OPTS		:= -M $(BOARD) -smp $(CPUS) -m $(EMU_RAM_SIZE) -nographic -no-reboot

ifneq ($(DTB),)
EMU_OPTS		+= -dtb $(DTB)
endif

.PHONY: run dbg gdb

run: EMU_OPTS		+= -kernel $(target-dir)/jrinx -bios $(OPENSBI_FW_JUMP) -append "$(EMU_ARGS)"
run: opensbi
	$(CMD_PREFIX)$(EMU) $(EMU_OPTS)

dbg: EMU_OPTS		+= -s -S
dbg: run

GDB			?= gdb-multiarch
GDB_EVAL_CMD		:= -ex 'target remote localhost:1234'

ifneq ($(GDB_SYM_FILE),)
GDB_EVAL_CMD		+= \
			-ex 'set confirm off' \
			-ex 'symbol-file $(GDB_SYM_FILE)' \
			-ex 'set confirm on'
endif

gdb:
	$(CMD_PREFIX)$(GDB) $(GDB_EVAL_CMD) $(target-dir)/jrinx

.PHONY: dumpdts dumpdtb
dumpdts: dumpdtb
	$(CMD_PREFIX)$(DTC) -I dtb -O dts $(BOARD).dtb -o $(BOARD).dts

dumpdtb: EMU_OPTS	+= -M $(BOARD),dumpdtb=$(BOARD).dtb
dumpdtb:
	$(CMD_PREFIX)$(EMU) $(EMU_OPTS)

.PHONY: check-style fix-style
check-style:
	$(CMD_PREFIX)$(CHECKSTYLE)

fix-style:
	$(CMD_PREFIX)$(FIXSTYLE)

.PHONY: register-git-hooks
register-git-hooks:
	$(CMD_PREFIX)ln -sf ../../scripts/pre-commit .git/hooks/pre-commit

.PHONY: cloc
cloc:
	$(CMD_PREFIX)$(CLOC) .

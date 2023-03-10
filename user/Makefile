USER_APPS	:= bare
USER_LDS	:= user.ld

.PHONY: all $(USER_APPS)

all: $(USER_APPS)

.ONESHELL:
$(USER_APPS):
$(USER_APPS): export USER_LDS_FILE := ../$(USER_LDS)
$(USER_APPS): $(USER_LDS)
	export BINTOC_PREFIX=$@
	if [ ! -f $@/Makefile ]; then
		$(MAKE) -C $@ -f $(BUILD_ROOT_DIR)/mk/auto-make-user.mk
	else
		$(MAKE) -C $@
	fi

INCLUDES	:= -I $(BUILD_ROOT_DIR)/include

include $(BUILD_ROOT_DIR)/mk/compile.mk

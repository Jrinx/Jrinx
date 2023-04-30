USER_LIBS		:= os sys lib
USER_APPS		:= app test

ifeq ($(BUILD_3RD_PARTY), y)
	USER_APPS	+= third-party
endif

.PHONY: all $(USER_APPS) $(USER_LIBS)

all: $(USER_APPS)
.ONESHELL:
$(USER_APPS): export USER_LINK_LIBS := $(addsuffix /*.o, $(abspath $(USER_LIBS)))
$(USER_APPS): $(USER_LIBS)
	export BINTOC_PREFIX=$@
	if [ ! -f $@/Makefile ]; then
		$(MAKE) -C $@ -f $(BUILD_ROOT_DIR)/mk/auto-make-user.mk
	else
		$(MAKE) -C $@
	fi

$(USER_LIBS):
	$(MAKE) -C $@ -f $(BUILD_ROOT_DIR)/mk/auto-make.mk

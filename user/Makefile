USER_LIBS	:= os sys lib
USER_BARE_APPS	:= bare
USER_APPS	:= general

.PHONY: all $(USER_BARE_APPS) $(USER_APPS) $(USER_LIBS)

all: $(USER_BARE_APPS) $(USER_APPS)
.ONESHELL:


$(USER_APPS): export USER_LINK_LIBS := $(addsuffix /*.o, $(abspath $(USER_LIBS)))
$(USER_APPS): $(USER_LIBS)

$(USER_BARE_APPS) $(USER_APPS):
	export BINTOC_PREFIX=$@
	if [ ! -f $@/Makefile ]; then
		$(MAKE) -C $@ -f $(BUILD_ROOT_DIR)/mk/auto-make-user.mk
	else
		$(MAKE) -C $@
	fi

$(USER_LIBS):
	$(MAKE) -C $@ -f $(BUILD_ROOT_DIR)/mk/auto-make.mk

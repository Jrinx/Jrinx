USER_APPS	:= bare

.PHONY: all $(USER_APPS)

all: $(USER_APPS)
.ONESHELL:
$(USER_APPS):
	export BINTOC_PREFIX=$@
	if [ ! -f $@/Makefile ]; then
		$(MAKE) -C $@ -f $(BUILD_ROOT_DIR)/mk/auto-make-user.mk
	else
		$(MAKE) -C $@
	fi

KERN_MODULES := $(dir $(wildcard ./*/))

.PHONY: all $(KERN_MODULES)

all: $(KERN_MODULES)

$(KERN_MODULES):
	$(MAKE) -C $@ $(shell \
	if [ ! -f $@/Makefile ]; then \
		echo '-f $(BUILD_ROOT_DIR)/mk/auto-make.mk'; \
	fi)

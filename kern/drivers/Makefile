INCLUDES := -I../../include

include ../../mk/compile.mk ../../mk/auto-tar.mk

DRIVERS := $(dir $(wildcard ./*/))

.PHONY: $(DRIVERS)
tar:: $(DRIVERS)

$(DRIVERS):
	$(MAKE) -C $@ $(shell \
	if [ ! -f $@/Makefile ]; then \
		echo '-f $(BUILD_ROOT_DIR)/mk/auto-make.mk'; \
	fi)

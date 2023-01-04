INCLUDES := -I../../include

include ../../mk/compile.mk ../../mk/auto-tar.mk

DRIVERS := $(dir $(wildcard ./*/))

.PHONY: $(DRIVERS)
tar:: $(DRIVERS)

$(DRIVERS):
	$(MAKE) -C $@

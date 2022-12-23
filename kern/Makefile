KERN_MODULES := $(dir $(wildcard ./*/))

.PHONY: all $(KERN_MODULES)

all: $(KERN_MODULES)

$(KERN_MODULES):
	$(MAKE) -C $@

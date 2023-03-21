include $(BUILD_ROOT_DIR)/mk/silent.mk

INCLUDES	+= -I$(BUILD_ROOT_DIR)/include

.ONESHELL:
%.i: %.c
	$(CPP) $(CFLAGS) $(INCLUDES) -E -P $< > $@

%.i: %.S
	$(CPP) $(CFLAGS) $(INCLUDES) -E -P $< > $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.b: SHELL := $(shell which bash)
%.b: %.o user.ld
	shopt -s nullglob globstar
	$(LD) $(LDFLAGS) -T user.ld -o $@ $< $(USER_LINK_LIBS) \
		$(addsuffix /**/*.o, $(addprefix $(BUILD_ROOT_DIR)/, $(LIB_MODULES)))

%.b.c: %.b
ifneq ($(BINTOC_PREFIX),)
	$(BINTOC) $^ -o $@ -p $(BINTOC_PREFIX)
else
	$(BINTOC) $^ -o $@
endif

%.x: %.b.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.ld: %.ld.S
	$(CPP) $(CFLAGS) $(INCLUDES) -E -P -x c $< > $@

vpath %.ld.S ./ $(BUILD_ROOT_DIR)/user/

.PRECIOUS: %.b %.b.c %.o

include $(BUILD_ROOT_DIR)/mk/silent.mk

%.i: %.c
	$(CPP) $(CFLAGS) $(INCLUDES) -E -P $< > $@

%.i: %.S
	$(CPP) $(CFLAGS) $(INCLUDES) -E -P $< > $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

USER_LD_FILE = user.ld

%.b: %.o $(USER_LD_FILE)
	[ -z "$^" ] && exit 59 || true
	$(LD) $(LDFLAGS) -T $(USER_LD_FILE) -o $@ $< $(USER_LINK_LIBS)

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

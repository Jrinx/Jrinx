include $(BUILD_ROOT_DIR)/mk/silent.mk

%.i: %.c
	$(CPP) $(CFLAGS) $(INCLUDES) -E -P $< > $@

%.i: %.S
	$(CPP) $(CFLAGS) $(INCLUDES) -E -P $< > $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.b: $(USER_OBJS)
	[ -z "$^" ] && exit 59 || true
	$(LD) $(LDFLAGS) -T $(USER_LDS_FILE) -o $@ $^

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

.PRECIOUS: %.b %.b.c %.o

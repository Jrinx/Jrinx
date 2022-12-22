include $(addsuffix silent.mk,$(dir $(lastword $(MAKEFILE_LIST))))

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.ld: %.ld.S
	$(CPP) $(CFLAGS) $(INCLUDES) -E -P -x c $< > $@

.gdbinit: .gdbinit.S
	$(CPP) $(CFLAGS) $(INCLUDES) -E -P -x c $< > $@

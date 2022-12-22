include $(addsuffix silent.mk,$(dir $(lastword $(MAKEFILE_LIST))))

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.ld: %.lds
	$(CPP) $(INCLUDES) -E -P -x c $< > $@

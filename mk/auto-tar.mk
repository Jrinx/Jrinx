.PHONY: tar
tar: $(patsubst %.c,%.o,$(wildcard *.c)) $(patsubst %.S,%.o,$(wildcard *.S))

COMPILE_TARGETS		:= $(patsubst %.c,%.o,$(wildcard *.c)) \
			$(patsubst %.S,%.o,$(wildcard *.S))

ifeq ($(CHECK_PREPROC),y)
	COMPILE_TARGETS	+= $(patsubst %.c,%.i,$(wildcard *.c)) \
			$(patsubst %.S,%.i,$(wildcard *.S))
endif

.PHONY: tar
tar:: $(COMPILE_TARGETS)

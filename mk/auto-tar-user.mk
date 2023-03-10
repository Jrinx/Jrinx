USER_OBJS		:= $(patsubst %.c,%.o,$(wildcard *.c)) \
			$(patsubst %.S,%.o,$(wildcard *.S))

COMPILE_TARGETS		:= $(patsubst %.c,%.x,$(wildcard *.c)) \
			$(patsubst %.S,%.x,$(wildcard *.S))

ifeq ($(CHECK_PREPROC),y)
	COMPILE_TARGETS	+= $(patsubst %.c,%.i,$(wildcard *.c)) \
			$(patsubst %.S,%.i,$(wildcard *.S))
endif

.PHONY: tar
tar:: $(COMPILE_TARGETS)

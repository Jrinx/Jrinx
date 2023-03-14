COMPLIE_TARGETS		:= $(patsubst %.c, %.o, $(wildcard *.c)) \
			$(patsubst %.S, %.o, $(wildcard *.S))

INCLUDES		:= -I$(BUILD_ROOT_DIR)/include

all: $(COMPLIE_TARGETS)

include $(BUILD_ROOT_DIR)/mk/compile.mk

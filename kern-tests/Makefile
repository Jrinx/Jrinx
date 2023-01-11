INCLUDES		:= -I../include

include ../mk/compile.mk

COMPILE_TARGETS		:= testkits.o

ifneq ($(TEST_NAME),)

ifeq ($(wildcard $(TEST_NAME).c),)
$(error test file '$(TEST_NAME)' not found)
endif
COMPILE_TARGETS		+= $(TEST_NAME).o

endif

.PHONY: tar
tar:: $(COMPILE_TARGETS)

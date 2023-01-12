INCLUDES		:= -I../include

include ../mk/compile.mk

COMPILE_TARGETS		:= testkits.o

ifneq ($(TEST),)

ifeq ($(wildcard $(TEST).c),)
$(error test file '$(TEST)' not found)
endif
COMPILE_TARGETS		+= $(TEST).o

endif

.PHONY: tar
tar:: $(COMPILE_TARGETS)

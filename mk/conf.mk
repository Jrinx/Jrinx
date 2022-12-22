ifeq ($(CONFIG_RELEASE),y)
	CFLAGS		+= -O2
	LDFLAGS		+= -O --gc-sections
else
	CFLAGS		+= -O0 -g -ggdb
endif

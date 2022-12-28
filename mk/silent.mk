CC	= @echo '  CC      '$@; $(CROSS_COMPILE)gcc
CPP	= @echo '  CPP     '$@; $(CROSS_COMPILE)cpp
LD	= @echo '  LINK    '$@; $(CROSS_COMPILE)ld
OBJDUMP = $(CROSS_COMPILE)objdump
OBJCOPY = $(CROSS_COMPILE)objcopy

IMPERAS_HOME := $(shell getpath.exe "$(IMPERAS_HOME)")
include $(IMPERAS_HOME)/bin/Makefile.include

#
#  Makefile for Cross Compiling an appication for a target processor type
#

# Various Cross compiler setups, Default or1k
CROSS?=OR1K
-include $(IMPERAS_HOME)/lib/$(IMPERAS_ARCH)/CrossCompiler/$(CROSS).makefile.include
ifeq ($($(CROSS)_CC),)
    IMPERAS_ERROR := $(error "Please install the $(CROSS) toolchain")
endif

IMPERAS_CC += -std=gnu99

ASRC = application.c test-regs.c test-remap.c simple-colorchange.c
AEXE = $(patsubst %.c,%.$(CROSS).elf,$(ASRC))

all: $(AEXE)

%.$(CROSS).elf: %.o
	$(V)  echo "Linking $@"
	$(V) $(IMPERAS_LINK) -o $@ $< $(IMPERAS_LDFLAGS) -lm

%.o: %.c
	$(V)  echo "Compiling $<"
	$(V) $(IMPERAS_CC) -c -o $@ $<

clean:
	- rm -f *.elf *.o
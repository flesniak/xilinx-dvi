IMPERAS_HOME := $(shell getpath.exe "$(IMPERAS_HOME)")
include $(IMPERAS_HOME)/bin/Makefile.include

#
#  Makefile for Cross Compiling an appication for a target processor type
#

# Various Cross compiler setups, Default or1k
CROSS?=MICROBLAZE
-include $(IMPERAS_HOME)/lib/$(IMPERAS_ARCH)/CrossCompiler/$(CROSS).makefile.include
ifeq ($($(CROSS)_CC),)
    IMPERAS_ERROR := $(error "Please install the $(CROSS) toolchain")
endif

IMPERAS_CFLAGS += -std=gnu99 -Os -mcpu=v8.20.a
IMPERAS_LDFLAGS += -Wl,lscript.ld

ASRC = rainbow.c test-regs.c test-remap.c simple-colorchange.c test-iesr.c test-configure.c
ASRCINC?=microblaze_io.c

AINCOBJ=$(patsubst %.c,%.$(CROSS).o,$(ASRCINC))
AEXE = $(patsubst %.c,%.$(CROSS).elf,$(ASRC))

all: $(AEXE)

%.$(CROSS).elf: %.$(CROSS).o $(AINCOBJ)
	$(V)  echo "Linking $@"
	$(V) $(IMPERAS_LINK) -o $@ $^ $(IMPERAS_LDFLAGS) -lm

%.$(CROSS).o: %.c
	$(V)  echo "Compiling $<"
	$(V) $(IMPERAS_CC) $(IMPERAS_CFLAGS) -c -o $@ $<

clean:
	- rm -f *.elf *.o

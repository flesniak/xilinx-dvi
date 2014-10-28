ifeq ($(origin IMPERAS_HOME), undefined)
  $(error IMPERAS_HOME is not set. Run /opt/set_ovp_env.sh)
endif
include $(IMPERAS_HOME)/bin/Makefile.include

SRC = semihost.c sdl.c #dlo.c
LIB = model.so
CFLAGS =  $(IMPERAS_VMIINC) -fPIC -I/usr/include/SDL -std=gnu99 -g -Wall -Werror -O2
LDFLAGS = $(SIM_LDFLAGS) $(IMPERAS_VMISTUBS) -shared -lSDL -pthread

ifeq ($(USE_DLO),1)
	LDFLAGS += -ldlo
	SRC += dlo.c
else
	CFLAGS += -DUSE_DLO
endif

SRCOBJ = $(patsubst %.c,%.o,$(SRC))

.PHONY : all clean

all : $(LIB)

$(LIB) : $(SRCOBJ)
	gcc -o $(LIB) $^ $(LDFLAGS)

$(SRCOBJ) : $(SRC)
	gcc $(CFLAGS) -c -o $@ $(@:%.o=%.c)

clean :
	@rm -f $(SRCOBJ) $(LIB)

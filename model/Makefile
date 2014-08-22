IMPERAS_HOME := $(shell getpath.exe "$(IMPERAS_HOME)")
CFLAGS  = -I$(IMPERAS_LIB)/External/include/SDL
CFLAGS += -I../../../../../ovpworld.org/modelSupport/sdl/1.0/model -std=gnu99
LDFLAGS = -L$(IMPERAS_LIB)/External/lib -lSDL -lpthread -ldlo
include $(IMPERAS_HOME)/ImperasLib/buildutils/Makefile.host

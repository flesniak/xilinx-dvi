CFLAGS = -std=gnu99 -Wall -Wextra -O2

all : dlotest sdltest

dlotest : dlotest.c
	gcc -o $@ $^ $(CFLAGS) -ldlo

sdltest : sdltest.c
	gcc -o $@ $^ $(CFLAGS) -lSDL -I/usr/include/SDL

clean :
	@rm -f dlotest sdltest

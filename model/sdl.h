#ifndef SDL_H
#define SDL_H

#include <SDL.h>
#include <stdbool.h>

typedef struct sdlObjectS {
  SDL_Surface* surface;
  SDL_Surface* tftSurface;
  int scanDirection;
} sdlObject;

void sdlInit(sdlObject* object, unsigned char* framebuffer, bool bigEndian);
void sdlFinish(sdlObject* object);
void sdlUpdate(sdlObject* object);
void sdlConfigure(sdlObject* object, int scanDirection);

void surfDump(SDL_Surface* surf, unsigned int n);

#endif //SDL_H

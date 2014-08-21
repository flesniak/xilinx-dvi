#ifndef SDL_H
#define SDL_H

#include <SDL.h>

typedef struct sdlObjectS {
  SDL_Surface* surface;
  SDL_Surface* tftSurface;
} sdlObject;

void sdlInit(sdlObject* object, unsigned char* framebuffer);
void sdlFinish(sdlObject* object);
void sdlUpdate(sdlObject* object);

void surfDump(SDL_Surface* surf, unsigned int n);

#endif //SDL_H

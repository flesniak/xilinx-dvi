#ifndef DLO_H
#define DLO_H

#include <libdlo.h>

typedef struct dloObjectS {
  dlo_dev_t dev;
  dlo_mode_t* mode;
  dlo_fbuf_t fbuf;
} dloObject;

void dloInit(dloObject* object, unsigned char* framebuffer);
void dloFinish(dloObject* object);
void dloUpdate(dloObject* object);

#endif //SDL_H

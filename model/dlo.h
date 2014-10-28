#ifndef DLO_H
#define DLO_H

#include "../dvi-mem.h"

#include <libdlo.h>
#include <stdint.h>

typedef struct dloObjectS {
  dlo_dev_t dev;
  dlo_mode_t* mode;
  dlo_fbuf_t fbuf;
  uint32_t* framebuffer;
  int scanDirection;
} dloObject;

void dloInit(dloObject* object, unsigned char* framebuffer);
void dloFinish(dloObject* object);
void dloUpdate(dloObject* object);
void dloConfigure(dloObject* object, int scanDirection);

void dloConvertPixels(dloObject* object);

#endif //SDL_H

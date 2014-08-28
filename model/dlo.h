#ifndef DLO_H
#define DLO_H

#include "../dvi-mem.h"

#ifndef NO_DLO
#include <libdlo.h>

typedef struct dloObjectS {
  dlo_dev_t dev;
  dlo_mode_t* mode;
  dlo_fbuf_t fbuf;
  uint32_t* framebuffer;
  int scanDirection;
} dloObject;
#else
typedef void dloObject;
#endif

void dloInit(dloObject* object, unsigned char* framebuffer);
void dloFinish(dloObject* object);
void dloUpdate(dloObject* object);
void dloConfigure(dloObject* object, int scanDirection);

void dloConvertPixels(uint32_t* dst, uint32_t* src, int scanDirection, int pitch);

#endif //SDL_H

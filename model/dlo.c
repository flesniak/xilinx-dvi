#include "dlo.h"
#include "../dvi-mem.h"

#include <vmi/vmiMessage.h>
#include <byteswap.h>

#ifndef NO_DLO
void dloInit(dloObject* object, unsigned char* framebuffer) {
  vmiMessage("I", "TFT_SH", "Initializing DLO output module");

  dlo_init_t initFlags;
  dlo_retcode_t err = dlo_init(initFlags);
  if( err != dlo_ok )
    vmiMessage("F", "TFT_SH", "Failed to initialize DisplayLink library: %s\n", dlo_strerror(err));

  dlo_claim_t claimFlags;
  object->dev = dlo_claim_first_device(claimFlags, 5000); //dont use dlo_claim_default_device because semihost wont get any argc/argv
  if( !object->dev )
    vmiMessage("F", "TFT_SH", "Failed to claim DisplayLink device\n");

  object->mode = dlo_get_mode(object->dev);
  if( 0 ) { //activate to set display to 640x480
    dlo_mode_t mode = { .view = {.width = DVI_OUTPUT_WIDTH, .height = DVI_OUTPUT_HEIGHT, .bpp = object->mode->view.bpp, .base = 0}, .refresh = 0 };
    err = dlo_set_mode(object->dev, &mode);
    if( err != dlo_ok )
      vmiMessage("F", "TFT_SH", "Failed to set mode: %s\n", dlo_strerror(err));
  }

  //fill the screen black using native libdlo methods
  err = dlo_fill_rect(object->dev, 0, 0, DLO_RGB(0, 0, 0));
  if( err != dlo_ok )
    vmiMessage("F", "TFT_SH", "Failed to fill rect: %s\n", dlo_strerror(err));

  object->fbuf.width = 2*DVI_OUTPUT_WIDTH;
  object->fbuf.height = DVI_OUTPUT_HEIGHT;
  object->fbuf.fmt = dlo_pixfmt_abgr8888;
  object->fbuf.base = malloc(2*DVI_OUTPUT_WIDTH*DVI_OUTPUT_HEIGHT*DVI_VMEM_BYTES_PER_PIXEL);
  object->fbuf.stride = 2*DVI_OUTPUT_WIDTH;

  object->framebuffer = (uint32_t*)framebuffer;
  object->scanDirection = DVI_SCAN_TOP_BOTTOM;
}

void dloFinish(dloObject* object) {
  vmiMessage("I", "TFT_SH", "Finishing DLO output module");
  dlo_retcode_t err = dlo_release_device(object->dev);
  if( err != dlo_ok )
    vmiMessage("F", "TFT_SH", "Failed to release DisplayLink device: %s\n", dlo_strerror(err));

  dlo_final_t finalFlags;
  err = dlo_final(finalFlags);
  if( err != dlo_ok )
    vmiMessage("F", "TFT_SH", "Failed to free DisplayLink library: %s\n", dlo_strerror(err));

  free(object->fbuf.base);
}

void dloUpdate(dloObject* object) {
  dloConvertPixels(object->fbuf.base, object->framebuffer, object->scanDirection, object->fbuf.stride);
  dloConvertPixels(((uint32_t*)object->fbuf.base)+DVI_OUTPUT_WIDTH, object->framebuffer+DVI_VMEM_SIZE/4, object->scanDirection, object->fbuf.stride);
  dlo_bmpflags_t bmpFlags = {0};
  dlo_retcode_t err = dlo_copy_host_bmp(object->dev, bmpFlags, &object->fbuf, &object->mode->view, 0);
  if( err != dlo_ok )
    vmiMessage("W", "TFT_SH", "Failed to copy host bmp: %s\n", dlo_strerror(err));
}

void dloConfigure(dloObject* object, int scanDirection) {
  object->scanDirection = scanDirection;
}

void dloConvertPixels(uint32_t* dst, uint32_t* src, int scanDirection, int pitch) {
  for( unsigned int y = 0; y < DVI_OUTPUT_HEIGHT; y++ )
    for( unsigned int x = 0; x < DVI_OUTPUT_WIDTH; x++ ) {
      uint32_t* srcPixel = src+DVI_VMEM_WIDTH*y+x;
      uint32_t* dstPixel;
      if( scanDirection == DVI_SCAN_BOTTOM_TOP )
        dstPixel = dst+pitch*(DVI_OUTPUT_HEIGHT-y)-x-1;
      else
        dstPixel = dst+pitch*y+x;
      *dstPixel = *srcPixel >> 6;
    }
}
#else
void dloInit(dloObject* object, unsigned char* framebuffer) {}
void dloFinish(dloObject* object) {}
void dloUpdate(dloObject* object) {}
void dloConfigure(dloObject* object, int scanDirection) {}
void dloConvertPixels(dloObject* object) {}
#endif

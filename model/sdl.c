#include "sdl.h"
#include "../dvi-mem.h"

#include <byteswap.h>
#include <vmi/vmiMessage.h>
#include <vmi/vmiRt.h>

void sdlInit(sdlObject* object, unsigned char* framebuffer, bool bigEndian) {
  vmiMessage("I", "TFT_SH", "Initializing SDL output module");
  if( SDL_Init(SDL_INIT_VIDEO) )
    vmiMessage("F", "TFT_SH", "Couldn't initialize SDL: %s\n", SDL_GetError());

  object->surface = SDL_SetVideoMode(DVI_OUTPUT_WIDTH, DVI_OUTPUT_HEIGHT, 0, SDL_SWSURFACE);
  if( !object->surface )
    vmiMessage("F", "TFT_SH", "Couldn't initialize surface: %s\n", SDL_GetError());
  SDL_FillRect(object->surface, 0, SDL_MapRGB(object->surface->format, 0, 0, 0));
  SDL_UpdateRect(object->surface, 0, 0, 0, 0);
  SDL_WM_SetCaption("OVP SDL output", 0);

  //create a surface matching the pixel format of the xilinx tft controller using the supplied framebuffer as pixel storage
  //depending on the endianess, swap the color masks
  if( bigEndian )
    object->tftSurface = SDL_CreateRGBSurfaceFrom(framebuffer, DVI_VMEM_WIDTH, DVI_VMEM_HEIGHT, DVI_VMEM_BITS_PER_PIXEL, DVI_VMEM_SCANLINE_BYTES, DVI_VMEM_RMASK, DVI_VMEM_GMASK, DVI_VMEM_BMASK, 0);
  else
    object->tftSurface = SDL_CreateRGBSurfaceFrom(framebuffer, DVI_VMEM_WIDTH, DVI_VMEM_HEIGHT, DVI_VMEM_BITS_PER_PIXEL, DVI_VMEM_SCANLINE_BYTES, bswap_32(DVI_VMEM_RMASK), bswap_32(DVI_VMEM_GMASK), bswap_32(DVI_VMEM_BMASK), 0);
  if( !object->tftSurface )
    vmiMessage("F", "TFT_SH", "Couldn't initialize tft surface: %s\n", SDL_GetError());
  SDL_FillRect(object->tftSurface, 0, SDL_MapRGB(object->tftSurface->format, 0, 0, 0));
  SDL_UpdateRect(object->tftSurface, 0, 0, 0, 0);

  object->scanDirection = DVI_SCAN_TOP_BOTTOM;
}

void sdlFinish(sdlObject* object) {
  vmiMessage("I", "TFT_SH", "Finishing SDL output module");
  SDL_FreeSurface(object->tftSurface);
  SDL_FreeSurface(object->surface);
  SDL_Quit();
}

void sdlUpdate(sdlObject* object) {
  if( SDL_BlitSurface(object->tftSurface, 0, object->surface, 0) )
    vmiMessage("W", "TFT_SH", "SDL Surface blit failed");
  SDL_UpdateRect(object->surface, 0, 0, 0, 0);

  //event loop to support clicking the "close" button
  /*SDL_Event event;
  while( SDL_PollEvent(&event) )
    if( event.type == SDL_QUIT ) {
      vmiMessage("I", "TFT_SH", "SDL Quit event, stopping simulation");
      vmirtFinish(0);
    }*/
}

void sdlConfigure(sdlObject* object, int scanDirection) {
  object->scanDirection = scanDirection;
}

void surfDump(SDL_Surface* surf, unsigned int n) {
  while( n%4 )
    n++;
  printf("Surface dump first %d bytes:", n);
  unsigned int i;
  for(i=0; i<n; i+=4)
    printf(" 0x%02x%02x%02x%02x",
           ((unsigned char*)surf->pixels)[i],
           ((unsigned char*)surf->pixels)[i+1],
           ((unsigned char*)surf->pixels)[i+2],
           ((unsigned char*)surf->pixels)[i+3]);
  printf("\n");
}

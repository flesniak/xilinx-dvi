// Xilinx XPS TFT Controller
// Semihost part, handles intercepts

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <SDL.h>
#include <pthread.h>

#include <vmi/vmiMessage.h>
#include <vmi/vmiTypes.h>
#include <vmi/vmiOSAttrs.h>
#include <vmi/vmiOSLib.h>

#include "../dvi-mem.h"

typedef struct vmiosObjectS {
  vmiRegInfoCP resultLow;
  vmiRegInfoCP resultHigh;
  vmiRegInfoCP stackPointer;
  SDL_Surface* surface;
  SDL_Surface* tftSurface;
  Uns32 vmemBaseAddr;
  pthread_t redrawThread;
  int redrawThreadState;
} vmiosObject;

static void getArg(vmiProcessorP processor, vmiosObjectP object, Uns32 *index, void* result, Uns32 argSize) {
  memDomainP domain = vmirtGetProcessorDataDomain(processor);
  Uns32 spAddr;
  vmirtRegRead(processor, object->stackPointer, &spAddr);
  vmirtReadNByteDomain(domain, spAddr+*index+4, result, argSize, 0, MEM_AA_FALSE);
  *index += argSize;
}

#define GET_ARG(_PROCESSOR, _OBJECT, _INDEX, _ARG) getArg(_PROCESSOR, _OBJECT, &_INDEX, &_ARG, sizeof(_ARG))

inline static void retArg(vmiProcessorP processor, vmiosObjectP object, void *result) {
  Uns64 arg = *(Uns64*)result;
  vmirtRegWrite(processor, object->resultLow, &arg);
  arg >>= 32;
  vmirtRegWrite(processor, object->resultHigh, &arg);
}

static void* drawDisplay(void* objectV) {
  vmiosObjectP object = (vmiosObjectP)objectV;
  object->redrawThreadState = 1; //thread is running
  while( object->redrawThreadState == 1 ) {
    if( SDL_BlitSurface(object->tftSurface, 0, object->surface, 0) )
      vmiMessage("W", "TFT_SH", "Surface blit failed");
    SDL_UpdateRect(object->surface, 0, 0, 0, 0);
    usleep(1000000/DVI_TARGET_FPS);
  }
  object->redrawThreadState = 0; //thread is stopped
  pthread_exit(0);
}

static VMIOS_INTERCEPT_FN(initDisplay) {
  Uns32 index = 0;
  GET_ARG(processor, object, index, object->vmemBaseAddr);

  if( SDL_Init(SDL_INIT_VIDEO) )
    vmiMessage("F", "TFT_SH", "Couldn't initialize SDL: %s\n", SDL_GetError());

  object->surface = SDL_SetVideoMode(640, 480, 0, SDL_SWSURFACE);
  if( !object->surface )
    vmiMessage("F", "TFT_SH", "Couldn't initialize surface: %s\n", SDL_GetError());
  SDL_FillRect(object->surface, 0, SDL_MapRGB(object->surface->format, 0, 0, 0xff));
  SDL_UpdateRect(object->surface, 0, 0, 0, 0);

  //create a surface matching the pixel format of the xilinx tft controller
  object->tftSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, 1024, 512, 32, 0x00003f00, 0x003f0000, 0x3f000000, 0);
  if( !object->tftSurface )
    vmiMessage("F", "TFT_SH", "Couldn't initialize tft surface: %s\n", SDL_GetError());
  SDL_FillRect(object->tftSurface, 0, SDL_MapRGB(object->tftSurface->format, 0, 0, 0));
  SDL_UpdateRect(object->tftSurface, 0, 0, 0, 0);

  //map native memory (tftSurface pixel storage) into the pse memory domain
  memDomainP domain = vmirtGetProcessorDataDomain(processor);
  if( !vmirtMapNativeMemory(domain, object->vmemBaseAddr, object->vmemBaseAddr+DVI_VMEM_SIZE-1, object->tftSurface->pixels) )
  	vmiMessage("F", "TFT_SH", "Failed to map native vmem");

  pthread_create(&object->redrawThread, 0, drawDisplay, (void*)object);

  Uns32 success = 1;
  retArg(processor, object, &success); //return success
}

/*void surfDump(SDL_Surface* surf, unsigned int n) {
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
}*/

/*static VMIOS_INTERCEPT_FN(drawDisplay) {
  //memDomainP domain = vmirtGetProcessorDataDomain(processor);
  //(domain, object->vmemBaseAddr, object->tftSurface->pixels, 1024*512*4, 0, True);
  if( SDL_BlitSurface(object->tftSurface, 0, object->surface, 0) )
    vmiMessage("W", "TFT_SH", "Surface blit failed");
  SDL_UpdateRect(object->surface, 0, 0, 0, 0);
}*/

/*static VMIOS_INTERCEPT_FN(updateVmem) {
  Uns32 updateBase = 0, updateSize = 0, index = 0;
  GET_ARG(processor, object, index, updateBase);
  GET_ARG(processor, object, index, updateSize);

  //resized tftSurface to vmem size, thus we do not need to strip out the vmem hole
  const Uns32 offset = updateBase-object->vmemBaseAddr;
  const Uns32 y = offset / 4096; //offset in scanlines (pitch = BytesPerPixel*xResolution)
  const Uns32 x = (offset % 4096); //offset in bytes (4 BytesPerPixel)
  const Uns32 surfaceOffset = y*object->tftSurface->pitch + x;
  if( x > 640*4-1 || y > 479 ) {
    vmiMessage("W", "TFT_SH", "Write to vmem out of 640x480 bounds occured, discarding (x %d y %d offset %d)\n", x/4, y, surfaceOffset);
    return;
  } else
    vmiMessage("I", "TFT_SH", "pixel data written to x %d y %d (offset %d)\n", x, y, surfaceOffset);

  memDomainP domain = vmirtGetProcessorDataDomain(processor);
  SDL_LockSurface(object->surface);
  vmirtReadNByteDomain(domain, updateBase, object->tftSurface->pixels, updateSize, 0, True);
  SDL_UnlockSurface(object->surface);
}*/

static VMIOS_CONSTRUCTOR_FN(constructor) {
  vmiMessage("I" ,"TFT_SH", "Constructing");
  const char *procType = vmirtProcessorType(processor);
  memEndian endian = vmirtGetProcessorDataEndian(processor);

  if( strcmp(procType, "pse") )
      vmiMessage("F", "TFT_SH", "Processor must be a PSE (is %s)\n", procType);
  if( endian != MEM_ENDIAN_NATIVE )
      vmiMessage("F", "TFT_SH", "Host processor must same endianity as a PSE\n");

  //get register description and store to object for further use
  object->resultLow = vmirtGetRegByName(processor, "eax");
  object->resultHigh = vmirtGetRegByName(processor, "edx");
  object->stackPointer = vmirtGetRegByName(processor, "esp");

  //set redrawThreadState = 0 (0 = not running, 1 = running, 2 = stop when possible)
  object->redrawThreadState = 0;
}

static VMIOS_CONSTRUCTOR_FN(destructor) {
  vmiMessage("I" ,"TFT_SH", "Shutting down");
  object->redrawThreadState = 2; //set stop condition
  pthread_join(object->redrawThread, 0);
  SDL_FreeSurface(object->tftSurface);
  SDL_FreeSurface(object->surface);
}

vmiosAttr modelAttrs = {
    .versionString  = VMI_VERSION,                // version string (THIS MUST BE FIRST)
    .modelType      = VMI_INTERCEPT_LIBRARY,      // type
    .packageName    = "xps-tft",                  // description
    .objectSize     = sizeof(vmiosObject),        // size in bytes of object

    .constructorCB  = constructor,                // object constructor
    .destructorCB   = destructor,                 // object destructor

    .morphCB        = 0,                          // morph callback
    .nextPCCB       = 0,                          // get next instruction address
    .disCB          = 0,                          // disassemble instruction

    // -------------------          -------- ------  -----------------
    // Name                         Address  Opaque  Callback
    // -------------------          -------- ------  -----------------
    .intercepts = {
        {"initDisplay",             0,       True,   initDisplay            },
        //{"drawDisplay",             0,       True,   drawDisplay            },
        //{"updateVmem",              0,       True,   updateVmem             },
        {0}
    }
};

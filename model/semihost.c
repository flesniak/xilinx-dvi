// Xilinx XPS TFT Controller
// Semihost part, handles intercepts

#include <byteswap.h>
#include <stdbool.h>
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
#include <vmi/vmiPSE.h>

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
  bool bigEndianGuest;
  int output;
  int redrawMode;
  memDomainP realDomain;
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

inline static void drawDisplay(vmiosObjectP object) {
  if( SDL_BlitSurface(object->tftSurface, 0, object->surface, 0) )
    vmiMessage("W", "TFT_SH", "Surface blit failed");
  SDL_UpdateRect(object->surface, 0, 0, 0, 0);
}

static void* drawDisplayThread(void* objectV) {
  vmiosObjectP object = (vmiosObjectP)objectV;
  object->redrawThreadState = 1; //thread is running
  while( object->redrawThreadState == 1 ) {
    //surfDump(object->tftSurface, 64);
    drawDisplay(object);
    usleep(1000000/DVI_TARGET_FPS);
  }
  object->redrawThreadState = 0; //thread is stopped
  pthread_exit(0);
}

memDomainP getSimulatedVmemDomain(vmiProcessorP processor, char* name) {
  Addr lo, hi;
  Bool isMaster, isDynamic;
  memDomainP simDomain = vmipsePlatformPortAttributes(processor, DVI_VMEM_BUS_NAME, &lo, &hi, &isMaster, &isDynamic);
  if( !simDomain )
    vmiMessage("F", "TFT_SH", "Failed to obtain %s platform port", DVI_VMEM_BUS_NAME);
  return simDomain;
}

static VMIOS_INTERCEPT_FN(initDisplay) {
  Uns32 index = 0;
  GET_ARG(processor, object, index, object->bigEndianGuest);
  GET_ARG(processor, object, index, object->output);
  GET_ARG(processor, object, index, object->redrawMode);
  
  if( object->bigEndianGuest ) {
    vmiMessage("I", "TFT_SH", "working with big-endian guest");
    object->output = bswap_32(object->output);
    object->output = bswap_32(object->redrawMode);
  }

  if( object->output == DVI_OUTPUT_DLO )
    vmiMessage("W", "TFT_SH", "DLO output not implemented yet, falling back to SDL");

  if( SDL_Init(SDL_INIT_VIDEO) )
    vmiMessage("F", "TFT_SH", "Couldn't initialize SDL: %s\n", SDL_GetError());

  object->surface = SDL_SetVideoMode(640, 480, 0, SDL_SWSURFACE);
  if( !object->surface )
    vmiMessage("F", "TFT_SH", "Couldn't initialize surface: %s\n", SDL_GetError());
  SDL_FillRect(object->surface, 0, SDL_MapRGB(object->surface->format, 0, 0, 0));
  SDL_UpdateRect(object->surface, 0, 0, 0, 0);

  //create a surface matching the pixel format of the xilinx tft controller
  object->tftSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, 1024, 512, 32, 0x00003f00, 0x003f0000, 0x3f000000, 0);
  if( !object->tftSurface )
    vmiMessage("F", "TFT_SH", "Couldn't initialize tft surface: %s\n", SDL_GetError());
  SDL_FillRect(object->tftSurface, 0, SDL_MapRGB(object->tftSurface->format, 0, 0, 0));
  SDL_UpdateRect(object->tftSurface, 0, 0, 0, 0);

  object->realDomain = vmirtNewDomain("real", 32);
  getSimulatedVmemDomain(processor, DVI_VMEM_BUS_NAME); //just to check if VMEMBUS is connected
  if( !vmirtMapNativeMemory(object->realDomain, 0, DVI_VMEM_SIZE-1, object->tftSurface->pixels) )
  	vmiMessage("F", "TFT_SH", "Failed to map native vmem to semihost memory domain");

  if( object->redrawMode == DVI_REDRAW_PTHREAD ) {
    vmiMessage("I", "TFT_SH", "Launching redraw pthread");
    pthread_create(&object->redrawThread, 0, drawDisplayThread, (void*)object);
  }

  Uns32 success = 1;
  retArg(processor, object, &success); //return success
}

static VMIOS_INTERCEPT_FN(configureDisplay) {
  Uns32 enable = 0, scanDirection = 0, index = 0;
  GET_ARG(processor, object, index, enable);
  GET_ARG(processor, object, index, scanDirection);
  if( object->bigEndianGuest )
    object->bigEndianGuest = bswap_32(object->bigEndianGuest);
  vmiMessage("W", "TFT_SH", "configureDisplay unimplemented");
}

void mapExternalVmemLocal(vmiProcessorP processor, vmiosObjectP object, Uns32 newVmemAddress) {
  vmiMessage("I", "TFT_SH", "Mapping external vmem (new addr 0x%08x, old addr 0x%08x)", newVmemAddress, object->vmemBaseAddr);
  if( object->vmemBaseAddr ) {
    vmiMessage("I", "TFT_SH", "Unaliasing previously mapped memory at 0x%08x", object->vmemBaseAddr);
    memDomainP simDomain = getSimulatedVmemDomain(processor, DVI_VMEM_BUS_NAME);
    vmirtUnaliasMemory(simDomain, object->vmemBaseAddr, object->vmemBaseAddr+DVI_VMEM_SIZE-1);
    //vmirtMapMemory(simDomain, object->vmemBaseAddr, object->vmemBaseAddr+DVI_VMEM_SIZE-1, MEM_RAM);
    //NOTE this unalias command does not work as expected. OVPsim seems to not yet have a way to dynamically unmap vmipse-mapped memory
    //The memory will be unmapped completely and not re-mapped to the ordinary platform-initialized RAM
    //Perhaps try to get originally mapped domain, save it and map it back later on here?
  }
  vmipseAliasMemory(object->realDomain, DVI_VMEM_BUS_NAME, newVmemAddress, newVmemAddress+DVI_VMEM_SIZE-1);
  object->vmemBaseAddr = newVmemAddress;
}

static VMIOS_INTERCEPT_FN(mapExternalVmem) {
  Uns32 newVmemAddress = 0, index = 0;
  GET_ARG(processor, object, index, newVmemAddress);
  if( object->bigEndianGuest )
    newVmemAddress = bswap_32(newVmemAddress);
  mapExternalVmemLocal(processor, object, newVmemAddress);
}

static VMIOS_INTERCEPT_FN(redrawCallback) {
  drawDisplay(object);
}

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
  //TODO get endianess directly from the simulated processor instead of a formal? is this possible?

  //redrawThreadState (0 = not running, 1 = running, 2 = stop when possible)
  object->redrawThreadState = 0;
}

static VMIOS_CONSTRUCTOR_FN(destructor) {
  vmiMessage("I" ,"TFT_SH", "Shutting down");
  object->redrawThreadState = 2; //set stop condition
  pthread_join(object->redrawThread, 0);
  SDL_FreeSurface(object->tftSurface);
  SDL_FreeSurface(object->surface);
  SDL_Quit();
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
        {"configureDisplay",        0,       True,   configureDisplay       },
        {"mapExternalVmem",         0,       True,   mapExternalVmem        },
        {"redrawCallback",          0,       True,   redrawCallback         },
        {0}
    }
};

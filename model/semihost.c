// Xilinx XPS TFT Controller
// Semihost part, handles intercepts

#include <byteswap.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>

#include <vmi/vmiMessage.h>
#include <vmi/vmiOSAttrs.h>
#include <vmi/vmiOSLib.h>
#include <vmi/vmiPSE.h>
#include <vmi/vmiTypes.h>

#include "dlo.h"
#include "sdl.h"
#include "../dvi-mem.h"

typedef struct vmiosObjectS {
  //register descriptions for get/return arguments
  vmiRegInfoCP resultLow;
  vmiRegInfoCP resultHigh;
  vmiRegInfoCP stackPointer;
  //vmiProcessorP processor;

  //memory information for mapping
  memDomainP guestDomain;
  Uns32 vmemBaseAddr;
  unsigned char* framebuffer;

  //output mode parameters
  enum outputModuleE { sdl = DVI_OUTPUT_SDL, dlo = DVI_OUTPUT_DLO } outputModule;
  int redrawMode;
  pthread_t redrawThread;
  int redrawThreadState;

  //other peripheral parameters
  bool bigEndianGuest;
  int enableVsyncInterrupt;
  int vsyncState;
  int enableDisplay;
  int scanDirection;
  //ppmNetHandle vsyncInterrupt;
  vmipseNetHandle vsyncInterrupt;
  
  //output module structs
  dloObject* dlo;
  sdlObject* sdl;
} vmiosObject;

static void getArg(vmiProcessorP processor, vmiosObjectP object, Uns32 *index, void* result, Uns32 argSize) {
  memDomainP domain = vmirtGetProcessorDataDomain(processor);
  Uns32 spAddr;
  vmirtRegRead(processor, object->stackPointer, &spAddr);
  vmirtReadNByteDomain(domain, spAddr+*index+4, result, argSize, 0, MEM_AA_FALSE);
  *index += argSize;
}

#define GET_ARG(_PROCESSOR, _OBJECT, _INDEX, _ARG) getArg(_PROCESSOR, _OBJECT, &_INDEX, &_ARG, sizeof(_ARG))

inline static void retArg(vmiProcessorP processor, vmiosObjectP object, Uns64 result) {
  vmirtRegWrite(processor, object->resultLow, &result);
  result >>= 32;
  vmirtRegWrite(processor, object->resultHigh, &result);
}

inline static void drawDisplay(vmiosObjectP object) {
  switch( object->outputModule ) {
    case sdl :
      sdlUpdate(object->sdl);
      break;
    case dlo :
      dloUpdate(object->dlo);
      break;
  }
}

static void* drawDisplayThread(void* objectV) {
  vmiosObjectP object = (vmiosObjectP)objectV;
  object->redrawThreadState = 1; //thread is running
  while( object->redrawThreadState == 1 ) {
    if( object->enableDisplay ) {
      if( object->enableVsyncInterrupt )
        vmipseWriteNet(object->vsyncInterrupt, 0);
        //vmirtWriteNetPort(object->processor, object->vsyncInterrupt, 0);
      object->vsyncState = 0;
      drawDisplay(object);
      object->vsyncState = 1;
      if( object->enableVsyncInterrupt )
        vmipseWriteNet(object->vsyncInterrupt, 1);
        //vmirtWriteNetPort(object->processor, object->vsyncInterrupt, 1);
    }
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

void mapExternalVmemLocal(vmiProcessorP processor, vmiosObjectP object, Uns32 newVmemAddress) {
  object->vsyncState = 0; //reset vsync flag to indicate new address is not yet displayed
  vmiMessage("I", "TFT_SH", "Mapping external vmem (new addr 0x%08x, old addr 0x%08x)", newVmemAddress, object->vmemBaseAddr);
  if( object->vmemBaseAddr ) {
    vmiMessage("I", "TFT_SH", "Unaliasing previously mapped memory at 0x%08x", object->vmemBaseAddr);
    vmirtUnaliasMemory(object->guestDomain, object->vmemBaseAddr, object->vmemBaseAddr+DVI_VMEM_SIZE-1);
    vmirtMapMemory(object->guestDomain, object->vmemBaseAddr, object->vmemBaseAddr+DVI_VMEM_SIZE-1, MEM_RAM);
    vmirtWriteNByteDomain(object->guestDomain, object->vmemBaseAddr, object->framebuffer, DVI_VMEM_SIZE, 0, MEM_AA_FALSE);
    vmirtReadNByteDomain(object->guestDomain, newVmemAddress, object->framebuffer, DVI_VMEM_SIZE, 0, MEM_AA_FALSE);
  }
  vmirtMapNativeMemory(object->guestDomain, newVmemAddress, newVmemAddress+DVI_VMEM_SIZE-1, object->framebuffer);
  object->vmemBaseAddr = newVmemAddress;
}

static VMIOS_INTERCEPT_FN(initDisplay) {
  //initialize object struct
  Uns32 index = 0;
  object->vmemBaseAddr = 0;
  object->framebuffer = calloc(1, DVI_VMEM_SIZE);
  GET_ARG(processor, object, index, object->outputModule);
  GET_ARG(processor, object, index, object->redrawMode);
  object->redrawThread = 0;
  object->redrawThreadState = 0;
  GET_ARG(processor, object, index, object->bigEndianGuest);
  object->enableVsyncInterrupt = 0;
  object->vsyncState = 0;
  object->enableDisplay = DVI_DISPLAY_ENABLED;
  object->scanDirection = DVI_SCAN_TOP_BOTTOM;
  GET_ARG(processor, object, index, object->vsyncInterrupt);
  object->dlo = 0;
  object->sdl = 0;

  switch( object->outputModule ) {
    case sdl :
      object->sdl = calloc(1, sizeof(sdlObject));
      sdlInit(object->sdl, object->framebuffer);
      break;
    case dlo :
      object->dlo = calloc(1, sizeof(dloObject));
      dloInit(object->dlo, object->framebuffer);
      break;
    default :
      vmiMessage("F", "TFT_SH", "Unknown output module %d selected", (Uns32)object->outputModule);
  }

  object->guestDomain = getSimulatedVmemDomain(processor, DVI_VMEM_BUS_NAME); //check if VMEMBUS is connected

  //object->processor = processor; //store processor to set the vsync interrupt net on
  if( object->redrawMode == DVI_REDRAW_PTHREAD ) {
    vmiMessage("I", "TFT_SH", "Launching redraw pthread");
    pthread_create(&object->redrawThread, 0, drawDisplayThread, (void*)object);
  }

  retArg(processor, object, 1); //return success
}

static VMIOS_INTERCEPT_FN(configureDisplay) {
  Uns32 index = 0;
  GET_ARG(processor, object, index, object->enableDisplay);
  GET_ARG(processor, object, index, object->scanDirection);
  vmiMessage("I", "TFT_SH", "configureDisplay enable 0x%08x scanDirection 0x%08x", object->enableDisplay, object->scanDirection);
  switch( object->outputModule ) {
    case sdl :
      sdlConfigure(object->sdl, object->scanDirection);
      break;
    case dlo :
      dloConfigure(object->dlo, object->scanDirection);
      break;
  }
}

static VMIOS_INTERCEPT_FN(mapExternalVmem) {
  Uns32 newVmemAddress = 0, index = 0;
  GET_ARG(processor, object, index, newVmemAddress);
  mapExternalVmemLocal(processor, object, newVmemAddress);
}

static VMIOS_INTERCEPT_FN(redrawCallback) {
  drawDisplay(object);
}

static VMIOS_INTERCEPT_FN(enableVsyncInterrupt) {
  Uns32 enable = 0, index = 0;
  GET_ARG(processor, object, index, enable);
  object->enableVsyncInterrupt = enable;
}

static VMIOS_INTERCEPT_FN(getVsyncStatus) {
  retArg(processor, object, object->vsyncState);
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

  object->redrawThreadState = 0; //(0 = not running, 1 = running, 2 = stop when possible)
  object->enableVsyncInterrupt = 0;
  object->vsyncState = 0;
}

static VMIOS_CONSTRUCTOR_FN(destructor) {
  vmiMessage("I" ,"TFT_SH", "Shutting down");
  object->redrawThreadState = 2; //set stop condition
  pthread_join(object->redrawThread, 0);
  switch( object->outputModule ) {
    case sdl :
      sdlFinish(object->sdl);
      free(object->sdl);
      break;
    case dlo :
      dloFinish(object->dlo);
      free(object->dlo);
      break;
  }
  free(object->framebuffer);
  vmiMessage("I", "TFT_SH", "Shutdown complete");
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
        {"enableVsyncInterrupt",    0,       True,   enableVsyncInterrupt   },
        {"getVsyncStatus",          0,       True,   getVsyncStatus         },
        {0}
    }
};

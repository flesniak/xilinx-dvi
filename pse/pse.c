
////////////////////////////////////////////////////////////////////////////////
//
//                W R I T T E N   B Y   I M P E R A S   I G E N
//
//                             Version 20131018.0
//                          Tue Jul  1 14:14:49 2014
//
////////////////////////////////////////////////////////////////////////////////

#include "byteswap.h"
#include "peripheral/bhm.h"
#include "string.h"

#include "xps_tft.igen.h"
#include "../dvi-mem.h"

typedef _Bool bool;

#define TARGET_FPS 25

Uns32 initDisplay(Uns32 endianess, Uns32 output, Uns32 redrawMode) {
  bhmMessage("F", "TFT_PSE", "Failed to intercept : initDisplay()");
  return 0;
}

void closeDisplay() {
  bhmMessage("F", "TFT_PSE", "Failed to intercept : closeDisplay()");
}

void configureDisplay(bool enable, bool scanDirection) {
    bhmMessage("F", "TFT_PSE", "Failed to intercept : configureDisplay()");
}

void mapExternalVmem(Uns32 baseAddress) {
  bhmMessage("F", "TFT_PSE", "Failed to intercept : mapExternalVmem()");
}

void redrawCallback() {
  bhmMessage("F", "TFT_PSE", "Failed to intercept : redrawCallback()");
}

PPM_REG_READ_CB(readReg) {
  Uns32 reg = (Uns32)user;
  switch( reg ) {
    case 0 : //address register
      bhmMessage("I", "TFT_PSE", "Read from address register for %d bytes at 0x%08x", bytes, (Uns32)addr);
      return BUS0_AB0_data.AR.value;
      break;
    case 1 : //control register
      bhmMessage("I", "TFT_PSE", "Read from control register for %d bytes at 0x%08x", bytes, (Uns32)addr);
      return BUS0_AB0_data.CR.value;
      break;
    case 2 : //interrupt enable & status register
      bhmMessage("I", "TFT_PSE", "Read from interrupt register for %d bytes at 0x%08x", bytes, (Uns32)addr);
      return BUS0_AB0_data.IESR.value; //TODO either update IESR.value from withing semihost or request vsync state from semihost
      break;
    case 3 : //chrontel chip register
      bhmMessage("W", "TFT_PSE", "Unhandled read from CCR: addr 0x%08x %d bytes", (Uns32)addr, bytes);
      return BUS0_AB0_data.CCR.value;
      break;
    default:
      bhmMessage("W", "TFT_PSE", "Invalid uder data on readReg\n");
  }

  return 0;
}

PPM_REG_WRITE_CB(writeReg) {
  Uns32 reg = (Uns32)user;
  switch( reg ) {
    case 0 : //address register
      bhmMessage("I", "TFT_PSE", "VRAM address change to 0x%08x requested, remapping", data);
      BUS0_AB0_data.AR.value = data;
      mapExternalVmem(data);
      break;
    case 1 : //control register
      bhmMessage("I", "TFT_PSE", "Write to control register for %d bytes at 0x%08x data 0x%08x", bytes, (Uns32)addr, data);
      configureDisplay(data & 1, data >> 1 & 1);
      BUS0_AB0_data.CR.value = data;
      break;
    case 2 : //interrupt enable & status register
      BUS0_AB0_data.IESR.value = data;
      break;
    case 3 : //chrontel chip register
      bhmMessage("W", "TFT_PSE", "Unhandled write to CCR: addr 0x%08x data 0x%08x", (Uns32)addr, data);
      BUS0_AB0_data.CCR.value = data;
      break;
    default:
      bhmMessage("W", "TFT_PSE", "Invalid uder data on writeReg\n");
  }
}

PPM_CONSTRUCTOR_CB(constructor) {
  bhmMessage("I", "TFT_PSE", "Constructing");
  periphConstructor();

  bool endianess = bhmBoolAttribute("bigEndianGuest");

  char output[4];
  output[3] = 0;
  if( !bhmStringAttribute("output", output, 4) )
    bhmMessage("W", "TFT_PSE", "Reading output attribute failed");
  if( output[0] == 0 ) //default if no output attribute is set
    strcpy(output, "sdl");
  Uns32 outputNum = DVI_OUTPUT_SDL;
  if( !strcmp(output, "sdl") )
    outputNum = DVI_OUTPUT_SDL;
  else {
    if( !strcmp(output, "dlo") ) {
      outputNum = DVI_OUTPUT_DLO;
      bhmMessage("W", "TFT_PSE", "DLO output mode is not implemented yet");
    } else
      bhmMessage("F", "TFT_PSE", "Invalid output requested: %s", output);
  }

  Uns32 redrawMode = 0;
  if( !bhmIntegerAttribute("polledRedraw", &redrawMode) )
    bhmMessage("W", "TFT_PSE", "Reading polledRedraw attribute failed");

  bhmMessage("I", "TFT_PSE", "Initializing display initDisplay()");
  Uns32 success = initDisplay(endianess, outputNum, redrawMode);

  if( success )
    bhmMessage("I", "TFT_PSE", "Display initialized successfully");
  else
    bhmMessage("F", "TFT_PSE", "Failed to initialize display");

  if( endianess )
    BUS0_AB0_data.AR.value = bswap_32(DVI_VMEM_ADDRESS);
  else
    BUS0_AB0_data.AR.value = DVI_VMEM_ADDRESS;

  mapExternalVmem(BUS0_AB0_data.AR.value);
  
  if( redrawMode == DVI_REDRAW_PSE ) {
    bhmMessage("I", "TFT_PSE", "Using polled, synchronized PSE drawing");
    while( 1 ) {
      bhmWaitDelay(1000000/DVI_TARGET_FPS);
      redrawCallback();
    }
  }

  //bhmWaitEvent(bhmGetSystemEvent(BHM_SE_END_OF_SIMULATION)); //we do not have to wait at the end of this constructor
}

PPM_DESTRUCTOR_CB(destructor) {
  bhmMessage("I", "TFT_PSE", "Destructing");
}


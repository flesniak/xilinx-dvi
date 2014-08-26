
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
static bool bigEndianGuest;
static unsigned int redrawMode;

Uns32 initDisplay(Uns32 output, Uns32 redrawMode, Uns32 bigEndianGuest, ppmNetHandle vsyncInterrupt) {
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

void enableVsyncInterrupt(bool enabled) {
  bhmMessage("F", "TFT_PSE", "Failed to intercept : enableVsyncInterrupt()");
}

int getVsyncStatus() {
  bhmMessage("F", "TFT_PSE", "Failed to intercept : getVsyncStatus()");
  return 0;
}

PPM_REG_READ_CB(readReg) {
  Uns32 reg = (Uns32)user;
  switch( reg ) {
    case 0 : //address register
      bhmMessage("D", "TFT_PSE", "Read from address register for %d bytes at 0x%08x", bytes, (Uns32)addr);
      return bigEndianGuest ? bswap_32(BUS0_AB0_data.AR.value) : BUS0_AB0_data.AR.value;
      break;
    case 1 : //control register
      bhmMessage("D", "TFT_PSE", "Read from control register for %d bytes at 0x%08x", bytes, (Uns32)addr);
      return bigEndianGuest ? bswap_32(BUS0_AB0_data.CR.value) : BUS0_AB0_data.CR.value;
      break;
    case 2 : //interrupt enable & status register
      bhmMessage("D", "TFT_PSE", "Read from interrupt register for %d bytes at 0x%08x", bytes, (Uns32)addr);
      if( redrawMode == DVI_REDRAW_PTHREAD ) { //if pse parameters (formals) are changed, update index appropriately!
        Uns32 state = (BUS0_AB0_data.IESR.value & ~DVI_IESR_STATUS_MASK) | (getVsyncStatus() << DVI_IESR_STATUS_OFFSET);
        return bigEndianGuest ? bswap_32(state) : state;
      } else
        return bigEndianGuest ? bswap_32(BUS0_AB0_data.IESR.value) : BUS0_AB0_data.IESR.value;
      break;
    case 3 : //chrontel chip register
      bhmMessage("W", "TFT_PSE", "Unhandled read from CCR: addr 0x%08x %d bytes", (Uns32)addr, bytes);
      return bigEndianGuest ? bswap_32(BUS0_AB0_data.CCR.value) : BUS0_AB0_data.CCR.value;
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
      bhmMessage("D", "TFT_PSE", "Write to AR at 0x%08x data 0x%08x", (Uns32)addr, data);
      BUS0_AB0_data.AR.value = bigEndianGuest ? bswap_32(data) : data;
      mapExternalVmem(BUS0_AB0_data.AR.value);
      break;
    case 1 : //control register
      bhmMessage("I", "TFT_PSE", "Write to CR at 0x%08x data 0x%08x", (Uns32)addr, data);
      BUS0_AB0_data.CR.value = bigEndianGuest ? bswap_32(data) : data;
      configureDisplay(BUS0_AB0_data.CR.value & DVI_CR_EN_MASK, BUS0_AB0_data.CR.value & DVI_CR_DSC_MASK);
      break;
    case 2 : //interrupt enable & status register
      bhmMessage("W", "TFT_PSE", "Write to IESR at 0x%08x data 0x%08x", (Uns32)addr, data);
      BUS0_AB0_data.IESR.value = (bigEndianGuest ? bswap_32(data) : data) & ~DVI_IESR_STATUS_MASK; //mask out status bit, writes on it have no effect
      enableVsyncInterrupt(BUS0_AB0_data.IESR.value & DVI_IESR_INTENABLE_MASK);
      break;
    case 3 : //chrontel chip register
      bhmMessage("W", "TFT_PSE", "Unhandled write to CCR at 0x%08x data 0x%08x", (Uns32)addr, data);
      BUS0_AB0_data.CCR.value = bigEndianGuest ? bswap_32(data) : data;
      break;
    default:
      bhmMessage("W", "TFT_PSE", "Invalid uder data on writeReg\n");
  }
}

PPM_CONSTRUCTOR_CB(constructor) {
  bhmMessage("I", "TFT_PSE", "Constructing");
  periphConstructor();

  bigEndianGuest = bhmBoolAttribute("bigEndianGuest");

  char output[4];
  output[3] = 0;
  if( !bhmStringAttribute("output", output, 4) )
    bhmMessage("W", "TFT_PSE", "Reading output attribute failed");
  if( output[0] == 0 ) //default if no output attribute is set
    strcpy(output, DVI_OUTPUT_STR_SDL);
  Uns32 outputNum = DVI_OUTPUT_SDL;
  if( !strcmp(output, DVI_OUTPUT_STR_SDL) )
    outputNum = DVI_OUTPUT_SDL;
  else {
    if( !strcmp(output, DVI_OUTPUT_STR_DLO) )
      #ifndef NO_DLO
      outputNum = DVI_OUTPUT_DLO;
      #else
      bhmMessage("F", "TFT_PSE", "DLO output was disabled at compile time");
      #endif
    else
      bhmMessage("F", "TFT_PSE", "Invalid output requested: %s", output);
  }

  redrawMode = DVI_REDRAW_PTHREAD;
  if( !bhmIntegerAttribute("polledRedraw", &redrawMode) ) {
    bhmMessage("W", "TFT_PSE", "Reading polledRedraw attribute failed, using threaded redraw");
    redrawMode = DVI_REDRAW_PTHREAD;
  }

  bhmMessage("I", "TFT_PSE", "Initializing display initDisplay()");
  Uns32 success = initDisplay(outputNum, redrawMode, bigEndianGuest, handles.VSYNCINT);

  if( success )
    bhmMessage("I", "TFT_PSE", "Display initialized successfully");
  else
    bhmMessage("F", "TFT_PSE", "Failed to initialize display");

  //initialize registers to reset values
  BUS0_AB0_data.AR.value = DVI_VMEM_ADDRESS;
  BUS0_AB0_data.CR.value = DVI_CR_EN_MASK;
  BUS0_AB0_data.IESR.value = 0;
  BUS0_AB0_data.CCR.value = 0;

  mapExternalVmem(BUS0_AB0_data.AR.value);
  
  if( redrawMode == DVI_REDRAW_PSE ) {
    bhmMessage("I", "TFT_PSE", "Using polled, synchronized PSE drawing");
    while( 1 ) {
      BUS0_AB0_data.IESR.value |= DVI_IESR_STATUS_MASK; //set IESR vsync status flag
      redrawCallback();
      BUS0_AB0_data.IESR.value &= ~DVI_IESR_STATUS_MASK; //clear IESR vsync status flag
      bhmWaitDelay(1000000/DVI_TARGET_FPS);
    }
  }

  //bhmWaitEvent(bhmGetSystemEvent(BHM_SE_END_OF_SIMULATION)); //we do not have to wait at the end of this constructor
}

PPM_DESTRUCTOR_CB(destructor) {
  bhmMessage("I", "TFT_PSE", "Destructing");
}


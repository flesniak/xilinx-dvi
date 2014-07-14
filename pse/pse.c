
////////////////////////////////////////////////////////////////////////////////
//
//                W R I T T E N   B Y   I M P E R A S   I G E N
//
//                             Version 20131018.0
//                          Tue Jul  1 14:14:49 2014
//
////////////////////////////////////////////////////////////////////////////////

#include "peripheral/bhm.h"

#include "xps_tft.igen.h"

#define TARGET_FPS 25

Uns32 initDisplay(Uns32 vmemBaseAddr) {
  bhmMessage("F", "TFT_PSE", "Failed to intercept : initDisplay()");
  return 0;
}

/*Uns32 drawDisplay() {
  bhmMessage("F", "TFT_PSE", "Failed to intercept : drawDisplay(Uns64)");
  return 0;
}*/

void closeDisplay() {
  bhmMessage("F", "TFT_PSE", "Failed to intercept : closeDisplay(Uns64)");
}

void updateVmem(Uns32 baseAddress, Uns32 size) {
  bhmMessage("F", "TFT_PSE", "Failed to intercept : updateVmem(Uns32,Uns32)");
}

PPM_CONSTRUCTOR_CB(constructor) {
  bhmMessage("I", "TFT_PSE", "Constructing");
  periphConstructor();

  bhmMessage("I", "XPS_PSE", "Initializing display initDisplay()");
  Uns32 success = initDisplay((Uns32)handles.VMEMBUS);
  if( success )
    bhmMessage("I", "XPS_PSE", "Display initialized successfully");
  else
    bhmMessage("F", "XPS_PSE", "Failed to initialize display");

  /*while( success ) {
    bhmWaitDelay(1000000/TARGET_FPS);
    bhmMessage("I", "XPS_PSE", "Redrawing display drawDisplay()");
    success = drawDisplay();
  }*/
  bhmWaitEvent(bhmGetSystemEvent(BHM_SE_END_OF_SIMULATION) );
}

PPM_WRITE_CB(vmemChange) {
  updateVmem((Uns32)addr, bytes);
}

PPM_DESTRUCTOR_CB(destructor) {
  bhmMessage("I", "TFT_PSE", "Destructing");
}

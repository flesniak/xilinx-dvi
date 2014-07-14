
////////////////////////////////////////////////////////////////////////////////
//
//                W R I T T E N   B Y   I M P E R A S   I G E N
//
//                             Version 20131018.0
//                          Thu Jul  3 14:40:50 2014
//
////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////// Licensing //////////////////////////////////

// Open Source Apache 2.0

////////////////////////////////// Description /////////////////////////////////

// Xilinx XPS TFT Controller


#include "xps_tft.igen.h"
/////////////////////////////// Port Declarations //////////////////////////////

BUS0_AB0_dataT BUS0_AB0_data;

VMEMBUS_VMEMAB_dataT VMEMBUS_VMEMAB_data;

handlesT handles;

/////////////////////////////// Diagnostic level ///////////////////////////////

// Test this variable to determine what diagnostics to output.
// eg. if (diagnosticLevel > 0) bhmMessage("I", "xps_tft", "Example");

Uns32 diagnosticLevel;

/////////////////////////// Diagnostic level callback //////////////////////////

static void setDiagLevel(Uns32 new) {
    diagnosticLevel = new;
}

///////////////////////////// MMR Generic callbacks ////////////////////////////

static PPM_VIEW_CB(view32) {  *(Uns32*)data = *(Uns32*)user; }

static PPM_READ_CB(read_32) {  return *(Uns32*)user; }

static PPM_WRITE_CB(write_32) { *(Uns32*)user = data; }

//////////////////////////////// Bus Slave Ports ///////////////////////////////

PPM_WRITE_CB(vmemChange);

static void installSlavePorts(void) {
    handles.BUS0 = ppmCreateSlaveBusPort("BUS0", 16);
    if (!handles.BUS0) {
        bhmMessage("E", "PPM_SPNC", "Could not connect port 'BUS0'");
    }

    handles.VMEMBUS = ppmCreateSlaveBusPort("VMEMBUS", 4194304);
    if (!handles.VMEMBUS) {
        bhmMessage("E", "PPM_SPNC", "Could not connect port 'VMEMBUS'");
    }

    //ppmInstallChangeCallback(vmemChange, (void*)0x0, handles.VMEMBUS+0x0, 0x400000);
}

//////////////////////////// Memory mapped registers ///////////////////////////

static void installRegisters(void) {

    ppmCreateRegister("AR",
        0,
        handles.BUS0,
        0,
        4,
        read_32,
        write_32,
        view32,
        &(BUS0_AB0_data.AR.value),
        True
    );
    ppmCreateRegister("CR",
        0,
        handles.BUS0,
        4,
        4,
        read_32,
        write_32,
        view32,
        &(BUS0_AB0_data.CR.value),
        True
    );
    ppmCreateRegister("IESR",
        0,
        handles.BUS0,
        8,
        4,
        read_32,
        write_32,
        view32,
        &(BUS0_AB0_data.IESR.value),
        True
    );
    ppmCreateRegister("CCR",
        0,
        handles.BUS0,
        12,
        4,
        read_32,
        write_32,
        view32,
        &(BUS0_AB0_data.CCR.value),
        True
    );


}

////////////////////////////////// Constructor /////////////////////////////////

PPM_CONSTRUCTOR_CB(periphConstructor) {
    installSlavePorts();
    installRegisters();
}

///////////////////////////////////// Main /////////////////////////////////////

int main(int argc, char *argv[]) {
    diagnosticLevel = 0;
    bhmInstallDiagCB(setDiagLevel);
    constructor();

    bhmWaitEvent(bhmGetSystemEvent(BHM_SE_END_OF_SIMULATION));
    destructor();
    return 0;
}



////////////////////////////////////////////////////////////////////////////////
//
//                W R I T T E N   B Y   I M P E R A S   I G E N
//
//                             Version 20131018.0
//                          Thu Aug 21 18:02:43 2014
//
////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////// Licensing //////////////////////////////////

// Open Source Apache 2.0

////////////////////////////////// Description /////////////////////////////////

// Xilinx XPS TFT Controller


#include "xps_tft.igen.h"
/////////////////////////////// Port Declarations //////////////////////////////

BUS0_AB0_dataT BUS0_AB0_data;

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

//////////////////////////////// View callbacks ////////////////////////////////

static PPM_VIEW_CB(view_BUS0_AB0_AR) {
    *(Uns32*)data = BUS0_AB0_data.AR.value;
}

static PPM_VIEW_CB(view_BUS0_AB0_CR) {
    *(Uns32*)data = BUS0_AB0_data.CR.value;
}

static PPM_VIEW_CB(view_BUS0_AB0_IESR) {
    *(Uns32*)data = BUS0_AB0_data.IESR.value;
}

static PPM_VIEW_CB(view_BUS0_AB0_CCR) {
    *(Uns32*)data = BUS0_AB0_data.CCR.value;
}
//////////////////////////////// Bus Slave Ports ///////////////////////////////

static void installSlavePorts(void) {
    handles.BUS0 = ppmCreateSlaveBusPort("BUS0", 16);
    if (!handles.BUS0) {
        bhmMessage("E", "PPM_SPNC", "Could not connect port 'BUS0'");
    }

}

//////////////////////////// Memory mapped registers ///////////////////////////

static void installRegisters(void) {

    ppmCreateRegister("AR",
        0,
        handles.BUS0,
        0,
        4,
        readReg,
        writeReg,
        view_BUS0_AB0_AR,
        (void*)0x0,
        True
    );
    ppmCreateRegister("CR",
        0,
        handles.BUS0,
        4,
        4,
        readReg,
        writeReg,
        view_BUS0_AB0_CR,
        (void*)0x1,
        True
    );
    ppmCreateRegister("IESR",
        0,
        handles.BUS0,
        8,
        4,
        readReg,
        writeReg,
        view_BUS0_AB0_IESR,
        (void*)0x2,
        True
    );
    ppmCreateRegister("CCR",
        0,
        handles.BUS0,
        12,
        4,
        readReg,
        writeReg,
        view_BUS0_AB0_CCR,
        (void*)0x3,
        True
    );


}

/////////////////////////////////// Net Ports //////////////////////////////////

static void installNetPorts(void) {
// To write to this net, use ppmWriteNet(handles.VSYNCINT, value);

    handles.VSYNCINT = ppmOpenNetPort("VSYNCINT");

}

////////////////////////////////// Constructor /////////////////////////////////

PPM_CONSTRUCTOR_CB(periphConstructor) {
    installSlavePorts();
    installRegisters();
    installNetPorts();
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


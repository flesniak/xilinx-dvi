
////////////////////////////////////////////////////////////////////////////////
//
//                W R I T T E N   B Y   I M P E R A S   I G E N
//
//                             Version 20150205.0
//                          Mon May  4 12:16:44 2015
//
////////////////////////////////////////////////////////////////////////////////


#include "xps_tft.igen.h"
/////////////////////////////// Port Declarations //////////////////////////////

BUS0_AB0_dataT BUS0_AB0_data;

handlesT handles;

/////////////////////////////// Diagnostic level ///////////////////////////////

// Test this variable to determine what diagnostics to output.
// eg. if (diagnosticLevel >= 1) bhmMessage("I", "xps_tft", "Example");
//     Predefined macros PSE_DIAG_LOW, PSE_DIAG_MEDIUM and PSE_DIAG_HIGH may be used
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

    ppmCreateRegister("AB0_AR",
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
    ppmCreateRegister("AB0_CR",
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
    ppmCreateRegister("AB0_IESR",
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
    ppmCreateRegister("AB0_CCR",
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

    ppmDocNodeP doc1_node = ppmDocAddSection(0, "Licensing");
    ppmDocAddText(doc1_node, "Open Source Apache 2.0");
    ppmDocNodeP doc_11_node = ppmDocAddSection(0, "Description");
    ppmDocAddText(doc_11_node, "Xilinx XPS TFT Controller");

    diagnosticLevel = 0;
    bhmInstallDiagCB(setDiagLevel);
    constructor();

    bhmWaitEvent(bhmGetSystemEvent(BHM_SE_END_OF_SIMULATION));
    destructor();
    return 0;
}


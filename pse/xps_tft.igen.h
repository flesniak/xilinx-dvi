
////////////////////////////////////////////////////////////////////////////////
//
//                W R I T T E N   B Y   I M P E R A S   I G E N
//
//                             Version 20131018.0
//                          Tue Jul 22 14:36:17 2014
//
////////////////////////////////////////////////////////////////////////////////

#ifndef XPS_TFT_IGEN_H
#define XPS_TFT_IGEN_H

#ifdef _PSE_
#    include "peripheral/impTypes.h"
#    include "peripheral/bhm.h"
#    include "peripheral/ppm.h"
#else
#    include "hostapi/impTypes.h"
#endif

//////////////////////////////////// Externs ///////////////////////////////////

    extern Uns32 diagnosticLevel;

/////////////////////////// Register data declaration //////////////////////////

typedef struct BUS0_AB0_dataS { 
    union { 
        Uns32 value;
        struct {
            unsigned __pad0 : 20;
            unsigned BASEADDR : 11;
        } bits;
    } AR;
    union { 
        Uns32 value;
        struct {
            unsigned TDE : 1;
            unsigned DPS : 1;
        } bits;
    } CR;
    union { 
        Uns32 value;
        struct {
            unsigned ST : 1;
            unsigned __pad1 : 2;
            unsigned IE : 1;
        } bits;
    } IESR;
    union { 
        Uns32 value;
        struct {
            unsigned CD : 8;
            unsigned CA : 8;
            unsigned __pad16 : 15;
            unsigned ST : 1;
        } bits;
    } CCR;
} BUS0_AB0_dataT, *BUS0_AB0_dataTP;

/////////////////////////////// Port Declarations //////////////////////////////

extern BUS0_AB0_dataT BUS0_AB0_data;

#ifdef _PSE_
///////////////////////////////// Port handles /////////////////////////////////

typedef struct handlesS {
    void                 *BUS0;
} handlesT, *handlesTP;

extern handlesT handles;

////////////////////////////// Callback prototypes /////////////////////////////

PPM_REG_READ_CB(readReg);
PPM_REG_WRITE_CB(writeReg);
PPM_CONSTRUCTOR_CB(periphConstructor);
PPM_DESTRUCTOR_CB(periphDestructor);
PPM_CONSTRUCTOR_CB(constructor);
PPM_DESTRUCTOR_CB(destructor);

#endif

#endif

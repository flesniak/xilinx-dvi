
////////////////////////////////////////////////////////////////////////////////
//
//                W R I T T E N   B Y   I M P E R A S   I G E N
//
//                             Version 20131018.0
//                          Tue Jul 22 14:36:17 2014
//
////////////////////////////////////////////////////////////////////////////////

#ifndef XPS_TFT_MACROS_IGEN_H
#define XPS_TFT_MACROS_IGEN_H
/////////////////////////////////// Licensing //////////////////////////////////

// Open Source Apache 2.0

////////////////////////////////// Description /////////////////////////////////

// Xilinx XPS TFT Controller

// Before including this file in the application, define the indicated macros
// to fix the base address of each slave port.
// Set the macro 'BUS0' to the base of port 'BUS0'
#ifndef BUS0
    #error BUS0 is undefined.It needs to be set to the port base address
#endif
#define BUS0_AB0_AR    (BUS0 + 0x0)

#define BUS0_AB0_AR_BASEADDR   (0x7ff << 20)
#define BUS0_AB0_CR    (BUS0 + 0x4)

#define BUS0_AB0_CR_TDE   0x1
#define BUS0_AB0_CR_DPS   (0x1 << 1)
#define BUS0_AB0_IESR    (BUS0 + 0x8)

#define BUS0_AB0_IESR_ST   0x1
#define BUS0_AB0_IESR_IE   (0x1 << 3)
#define BUS0_AB0_CCR    (BUS0 + 0xc)

#define BUS0_AB0_CCR_CD   0xff
#define BUS0_AB0_CCR_CA   (0xff << 8)
#define BUS0_AB0_CCR_ST   (0x1 << 31)


#endif

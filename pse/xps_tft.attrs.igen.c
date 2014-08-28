
////////////////////////////////////////////////////////////////////////////////
//
//                W R I T T E N   B Y   I M P E R A S   I G E N
//
//                             Version 20131018.0
//                          Thu Aug 28 12:02:13 2014
//
////////////////////////////////////////////////////////////////////////////////


#ifdef _PSE_
#    include "peripheral/impTypes.h"
#    include "peripheral/bhm.h"
#    include "peripheral/ppm.h"
#else
#    include "hostapi/impTypes.h"
#endif


static ppmBusPort busPorts[] = {
    {
        .name            = "BUS0",
        .type            = PPM_SLAVE_PORT,
        .addrHi          = 0xfLL,
        .mustBeConnected = 1,
        .remappable      = 0,
        .description     = 0,
    },
    {
        .name            = "VMEMBUS",
        .type            = PPM_SLAVE_PORT,
        .addrHi          = 0x3fffffLL,
        .mustBeConnected = 1,
        .remappable      = 1,
        .description     = "Video memory, sized 2MB",
    },
    {
        .name            = "VMEMBUS2",
        .type            = PPM_SLAVE_PORT,
        .addrHi          = 0x3fffffLL,
        .mustBeConnected = 0,
        .remappable      = 1,
        .description     = "Second video memory, sized 2MB",
    },
    { 0 }
};

static PPM_BUS_PORT_FN(nextBusPort) {
    if(!busPort) {
        return busPorts;
    } else {
        busPort++;
    }
    return busPort->name ? busPort : 0;
}

static ppmNetPort netPorts[] = {
    {
        .name            = "VSYNCINT",
        .type            = PPM_OUTPUT_PORT,
        .mustBeConnected = 0,
        .description     = 0
    },
    { 0 }
};

static PPM_NET_PORT_FN(nextNetPort) {
    if(!netPort) {
        return netPorts;
    } else {
        netPort++;
    }
    return netPort->name ? netPort : 0;
}

static ppmParameter parameters[] = {
    {
        .name        = "bigEndianGuest",
        .type        = ppm_PT_BOOL,
        .description = 0,
    },
    {
        .name        = "output",
        .type        = ppm_PT_UNS32,
        .description = 0,
    },
    {
        .name        = "polledRedraw",
        .type        = ppm_PT_UNS32,
        .description = 0,
    },
    { 0 }
};

static PPM_PARAMETER_FN(nextParameter) {
    if(!parameter) {
        return parameters;
    } else {
        parameter++;
    }
    return parameter->name ? parameter : 0;
}

ppmModelAttr modelAttrs = {

    .versionString = PPM_VERSION_STRING,
    .type          = PPM_MT_PERIPHERAL,

    .busPortsCB    = nextBusPort,  
    .netPortsCB    = nextNetPort,  
    .paramSpecCB   = nextParameter,

    .vlnv          = {
        .vendor  = "itiv.kit.edu",
        .library = "peripheral",
        .name    = "xps_tft",
        .version = "1.0"
    },

    .family    = "itiv.kit.edu",           

};

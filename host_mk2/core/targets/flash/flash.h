#ifndef __FLASH_H__
#define __FLASH_H__

// Manufacturer IDs
#define MID_AMD        ( 0x0001 )
#define MID_AMIC       ( 0x0037 )
#define MID_CATALYST   ( 0x0031 )
#define MID_EON        ( 0x001C )
#define MID_FUJITSU    ( 0x0004 )
#define MID_INTEL      ( 0x0089 )
#define MID_MXIC       ( 0x00C2 )
#define MID_ST         ( 0x0020 )
#define MID_WINBOND    ( 0x00DA )

enum flashType : uint32_t {
    enUnkFlash     = 0,
    enOgFlash      = 1,
    enToggleFlash  = 2
};

typedef struct {
    const uint32_t        did;
    const flashType       type;
    const uint32_t        count;
    const uint32_t *const partitions;
} flashpart_t;

typedef struct {
    const flashpart_t *const parts;
    const uint32_t           count;
} dids_t;

typedef struct {
    const dids_t x8parts;
    const dids_t x16parts;
    const dids_t x32parts;
} didcollection_t;

#define partMacro(prt) \
    sizeof(prt) / sizeof((prt)[0]), (prt)

#define colMacro(col) \
    (col), sizeof(col) / sizeof((col)[0])

#include "maps_29.h"
#include "maps_generic.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AMD                       ( 0001 )

static const flashpart_t amd_x8[] = {
    { 0x00000025, enOgFlash          , partMacro(one64k)        }, // AM28F512
    { 0x000000A7, enOgFlash          , partMacro(one128k)       }, // AM28F010
    { 0x0000002A, enOgFlash          , partMacro(one256k)       }, // AM28F020

    { 0x00000020, enToggleFlash      , partMacro(eight16k)      }, // AM29F010
};

static const flashpart_t amd_x16[] = {
    { 0x00002223, enToggleFlash      , partMacro(am29f400bt)    }, // Trionic 7
    { 0x000022AB, enToggleFlash      , partMacro(am29f400bb)    },
    { 0x000022D6, enToggleFlash      , partMacro(am29f800bt)    },
    { 0x00002258, enToggleFlash      , partMacro(am29f800bb)    },
    { 0x00002281, enToggleFlash      , partMacro(am29bl802c)    }, // Trionic 8
};

static const didcollection_t amd_dids = {
    { colMacro(amd_x8)  },
    { colMacro(amd_x16) },
    { nullptr }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AMIC                      ( 0037 )

static const flashpart_t amic_x8[] = {
    { 0x000000A1, enToggleFlash      , partMacro(two32k)        }, // A29512
    { 0x000000A4, enToggleFlash      , partMacro(four32k)       }, // A29010
};

static const didcollection_t amic_dids = {
    { colMacro(amic_x8)  },
    { nullptr },
    { nullptr }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Catalyst (CSI)            ( 0031 )

static const flashpart_t catalyst_x8[] = {
    { 0x000000B8, enOgFlash          , partMacro(one64k)  }, // 28F512
    { 0x000000B4, enOgFlash          , partMacro(one128k) }, // 28F010
    { 0x000000BD, enOgFlash          , partMacro(one256k) }, // 28F020
};

static const didcollection_t catalyst_dids = {
    { colMacro(catalyst_x8)  },
    { nullptr },
    { nullptr }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EON                       ( 001C )

static const flashpart_t eon_x8[] = {
    { 0x00000021, enToggleFlash      , partMacro(four16k)       }, // EN29F512
    { 0x00000020, enToggleFlash      , partMacro(eight16k)      }, // EN29F010
    { 0x0000006E, enToggleFlash      , partMacro(eight16k)      }, // EN29LV010
};

static const didcollection_t eon_dids = {
    { colMacro(eon_x8)  },
    { nullptr },
    { nullptr }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Intel / TI                ( 0089 )

static const flashpart_t intel_x8[] = {
    { 0x000000B8, enOgFlash          , partMacro(one64k)  }, // 28F512
    { 0x000000B4, enOgFlash          , partMacro(one128k) }, // 28F010
    { 0x000000BD, enOgFlash          , partMacro(one256k) }, // 28F020
};

static const didcollection_t intel_dids = {
    { colMacro(intel_x8)  },
    { nullptr },
    { nullptr }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ST                        ( 0020 )

static const flashpart_t st_x8[] = {
    { 0x00000024, enToggleFlash      , partMacro(one64k)        }, // M29F512
    { 0x00000020, enToggleFlash      , partMacro(eight16k)      }, // M29F010
};

static const didcollection_t st_dids = {
    { colMacro(st_x8)  },
    { nullptr },
    { nullptr }
};

#endif

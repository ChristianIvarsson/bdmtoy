#ifndef __FLASH_H__
#define __FLASH_H__

// Manufacturer IDs
#define MID_AMD        ( 0x0001 )
#define MID_AMIC       ( 0x0037 )
#define MID_ATMEL      ( 0x001F )
#define MID_CATALYST   ( 0x0031 )
#define MID_EON        ( 0x001C )
#define MID_FUJITSU    ( 0x0004 )
#define MID_INTEL      ( 0x0089 )
#define MID_MXIC       ( 0x00C2 )
#define MID_ST         ( 0x0020 )
#define MID_WINBOND    ( 0x00DA )

enum flashType : uint32_t {
    enUnkFlash     =  0,
    enOgFlash      =  1,  // Regular 28f series. Later ones use a modified command set
    enToggleFlash  =  2,  // Most 29 and 39 series
    enPaged128     =  3,  // Flash with 128-byte pages (Atmel AT29C, Winbond W29C etc)
    enPaged256     =  4,  // Flash with 256-byte pages (Atmel AT29C, Winbond W29C etc)
    enOgFlashTyp2  = 11,  // For now. There's no support in the driver
};

typedef struct {
    const uint32_t        did;
    const char     *const pName; 
    const flashType       type;
    const uint32_t        count;
    const uint32_t *const partitions;
} flashpart_t;

typedef struct {
    const flashpart_t *const parts;
    const uint32_t           count;
} dids_t;

typedef struct {
    const char *const mName;
    const dids_t      x8parts;
    const dids_t      x16parts;
    const dids_t      x32parts;
} didcollection_t;

#define partMacro(prt) \
    sizeof(prt) / sizeof((prt)[0]), (prt)

#define colMacro(col) \
    (col), sizeof(col) / sizeof((col)[0])

#include "maps_28.h"
#include "maps_29.h"
#include "maps_generic.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AMD                       ( 0001 )

static const flashpart_t amd_x8[] = {
    { 0x00000025, "AM28F512"       , enOgFlash          , partMacro(one64k)        },
    { 0x000000A7, "AM28F010"       , enOgFlash          , partMacro(one128k)       },
    { 0x0000002A, "AM28F020"       , enOgFlash          , partMacro(one256k)       },
    { 0x00000020, "AM29F010"       , enToggleFlash      , partMacro(eight16k)      },
    { 0x000000B0, "AM29F002T"      , enToggleFlash      , partMacro(X002t)         },
    { 0x00000034, "AM29F002B"      , enToggleFlash      , partMacro(X002b)         },
};

static const flashpart_t amd_x16[] = {
    { 0x00002223, "AM29F400BT"     , enToggleFlash      , partMacro(am29f400bt)    }, // Trionic 7
    { 0x000022AB, "AM29F400BB"     , enToggleFlash      , partMacro(am29f400bb)    },
    { 0x000022D6, "AM29F800BT"     , enToggleFlash      , partMacro(am29f800bt)    },
    { 0x00002258, "AM29F800BB"     , enToggleFlash      , partMacro(am29f800bb)    },
    { 0x00002281, "AM29BL802C"     , enToggleFlash      , partMacro(am29bl802c)    }, // Trionic 8
};

static const didcollection_t amd_dids = {
    "AMD",
    { colMacro(amd_x8)  },
    { colMacro(amd_x16) },
    { nullptr }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AMIC                      ( 0037 )

static const flashpart_t amic_x8[] = {
    { 0x000000A1,"A29512"          , enToggleFlash      , partMacro(two32k)        },
    { 0x000000A4,"A29010"          , enToggleFlash      , partMacro(four32k)       },
    { 0x0000008C,"A29002T"         , enToggleFlash      , partMacro(X002t)         },
    { 0x0000000D,"A29002B"         , enToggleFlash      , partMacro(X002b)         },
};

static const didcollection_t amic_dids = {
    "AMIC",
    { colMacro(amic_x8)  },
    { nullptr },
    { nullptr }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ATMEL                     ( 001F )

// Note: All of this is very much a hack. There's no format support on this type of paged flash so it'll just pretend it erased everything and leave it to the page writer
// Thus, there's no need for a parition map (And typing out 256/512/1024 of them would just be silly)
static const flashpart_t atmel_x8[] = {
    { 0x0000005D, "AT29C512"       , enPaged128         , partMacro(one64k)        }, // (  64k.  512 128-byte pages )
    { 0x000000D5, "AT29C010"       , enPaged128         , partMacro(one128k)       }, // ( 128k. 1024 128-byte pages )
    { 0x000000DA, "AT29C020"       , enPaged256         , partMacro(one256k)       }, // ( 256k. 1024 256-byte pages ) 
};

static const didcollection_t atmel_dids = {
    "Atmel",
    { colMacro(atmel_x8)  },
    { nullptr },
    { nullptr }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Catalyst (CSI)            ( 0031 )

static const flashpart_t catalyst_x8[] = {
    { 0x000000B8, "28F512"         , enOgFlash          , partMacro(one64k)        },
    { 0x000000B4, "28F010"         , enOgFlash          , partMacro(one128k)       },
    { 0x000000BD, "28F020"         , enOgFlash          , partMacro(one256k)       },
};

static const didcollection_t catalyst_dids = {
    "Catalyst / CSI",
    { colMacro(catalyst_x8)  },
    { nullptr },
    { nullptr }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EON                       ( 001C )

static const flashpart_t eon_x8[] = {
    { 0x00000021, "EN29F512"       , enToggleFlash      , partMacro(four16k)       },
    { 0x00000020, "EN29F010"       , enToggleFlash      , partMacro(eight16k)      },
    { 0x0000006E, "EN29LV010"      , enToggleFlash      , partMacro(eight16k)      },
    { 0x00000092, "EN29F002T"      , enToggleFlash      , partMacro(X002t)         },
    { 0x00000097, "EN29F002B"      , enToggleFlash      , partMacro(X002b)         },
};

static const didcollection_t eon_dids = {
    "EON",
    { colMacro(eon_x8)  },
    { nullptr },
    { nullptr }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Fujitsu                   ( 0004 )

static const flashpart_t fujitsu_x8[] = {
    { 0x000000B0, "MBM29F002T"     , enToggleFlash      , partMacro(X002t)         },
    { 0x00000034, "MBM29F002B"     , enToggleFlash      , partMacro(X002b)         },
};

static const didcollection_t fujitsu_dids = {
    "Fujitsu",
    { colMacro(fujitsu_x8)  },
    { nullptr },
    { nullptr }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Intel / TI                ( 0089 )

static const flashpart_t intel_x8[] = {
    { 0x000000B8, "28F512"         , enOgFlash          , partMacro(one64k)        },
    { 0x000000B4, "28F010"         , enOgFlash          , partMacro(one128k)       },
    { 0x000000BD, "28F020"         , enOgFlash          , partMacro(one256k)       },
    { 0x0000007C, "28F002T"        , enOgFlashTyp2      , partMacro(intl28f002t)   },
    { 0x0000007D, "28F002B"        , enOgFlashTyp2      , partMacro(intl28f002b)   },
};

static const didcollection_t intel_dids = {
    "Intel or Texas Instruments",
    { colMacro(intel_x8)  },
    { nullptr },
    { nullptr }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MXIC                      ( 00C2 )

static const flashpart_t mxic_x8[] = {
    { 0x000000B0, "MX29F002T"      , enToggleFlash      , partMacro(X002t)         },
    { 0x00000034, "MX29F002B"      , enToggleFlash      , partMacro(X002b)         },
};

static const didcollection_t mxic_dids = {
    "MXIC",
    { colMacro(mxic_x8)  },
    { nullptr },
    { nullptr }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ST                        ( 0020 )

static const flashpart_t st_x8[] = {
    { 0x00000024, "M29F512"        , enToggleFlash      , partMacro(one64k)        },
    { 0x00000020, "M29F010"        , enToggleFlash      , partMacro(eight16k)      },
    { 0x000000B0, "M29F002BT/BNT"  , enToggleFlash      , partMacro(X002t)         },
    { 0x00000034, "M29F002BB"      , enToggleFlash      , partMacro(X002b)         },
};

static const didcollection_t st_dids = {
    "ST",
    { colMacro(st_x8)  },
    { nullptr },
    { nullptr }
};

#endif

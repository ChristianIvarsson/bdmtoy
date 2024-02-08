#ifndef __TARGETS_H__
#define __TARGETS_H__

// Some small notes
//
// bdmstuff::read() / write() tries to use index 0 as its source range so try to stick the main storage at that location
// For now, also try to stick main sram at index 1

class bdmstuff;
class iTarget;

// Host specific target enum
enum enTargTyp : uint32_t {
    typeError      = 0,
    typeCPU32      = 1,   // CPU32 (68k architecture)
};

enum enOpTyp : uint32_t {
    opFlash        = 0,
    opSRAM         = 1,
    opEEPROM       = 2,
};

// If someone knows a cleaner way, please do tell because I hate this "solution"
typedef iTarget *instan_t ( bdmstuff & );

typedef struct {
    enOpTyp  type;
    uint64_t address;
    uint32_t size;
} memory_t;

typedef struct {
    const instan_t  *const instantiator;
    const char      *const name;
    const char      *const info;
    const enTargTyp        type;
    const void      *const user;
    const uint32_t         regions;
    const memory_t         region[ 8 ]; // ISO C++ does not allow flexible arrays
} target_t;

class iTarget {

public:
    explicit iTarget() {}
    virtual ~iTarget() {}

    virtual bool init    ( const target_t *, const memory_t * ) { return false; }
    virtual bool write   ( const target_t *, const memory_t * ) { return false; }
    virtual bool read    ( const target_t *, const memory_t * ) { return false; }
};

static const target_t errDesc = { 
    nullptr, "Error", "Error", typeError
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Random CPU32 targets

// Instantiators
iTarget *instVolvoS60ACC( bdmstuff & );

static const target_t s60ACC = {
    instVolvoS60ACC,
    "Volvo S60/V70 ACC",
    "MC68376, CPU32",
    typeCPU32,
    nullptr,
    3,
    {
        { opFlash, 0x00000000, 0x00040000 }, // Flash     ( 256k )
        { opSRAM,  0x00200000, 0x00002000 }, // SRAM      ( 8k )
        { opSRAM,  0x00100000, 0x00000E00 }, // TPURAM    ( 3.5k )
    }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Trionic CPU32 family

// Instantiators
iTarget *instTrionic5( bdmstuff & );
iTarget *instTrionic7( bdmstuff & );
iTarget *instTrionic8( bdmstuff & );
iTarget *instTrionic8mcp( bdmstuff & );

static const target_t trionic_5_2 = {
    instTrionic5,
    "Trionic 5.2",
    "MC68332, CPU32",
    typeCPU32,
    nullptr,
    3,
    {
        { opFlash, 0x00000000, 0x00020000 }, // Flash     ( 128k )
        { opSRAM,  0x00200000, 0x00008000 }, // SRAM      ( 32k )
        { opSRAM,  0x00100000, 0x00000800 }, // TPURAM    ( 2k )
    }
};

static const target_t trionic_5_5 = {
    instTrionic5,
    "Trionic 5.5",
    "MC68332, CPU32",
    typeCPU32,
    nullptr,
    3,
    {
        { opFlash, 0x00000000, 0x00040000 }, // Flash     ( 256k )
        { opSRAM,  0x00200000, 0x00008000 }, // SRAM      ( 32k )
        { opSRAM,  0x00100000, 0x00000800 }, // TPURAM    ( 2k )
    }
};

static const target_t trionic_5_7 = {
    instTrionic5,
    "Trionic 5.5, 512K",
    "MC68332, CPU32",
    typeCPU32,
    nullptr,
    3,
    {
        { opFlash, 0x00000000, 0x00080000 }, // Flash     ( 512k )
        { opSRAM,  0x00200000, 0x00008000 }, // SRAM      ( 32k )
        { opSRAM,  0x00100000, 0x00000800 }, // TPURAM    ( 2k )
    }
};

static const target_t trionic_7_7 = {
    instTrionic7,
    "Trionic 7",
    "MC68339, CPU32",
    typeCPU32,
    nullptr,
    3,
    {
        { opFlash, 0x00000000, 0x00080000 }, // Flash     ( 512k )
        { opSRAM,  0x00200000, 0x00010000 }, // SRAM      ( 64k )
        { opSRAM,  0x00100000, 0x00000800 }, // TPURAM    ( 2k )
    }
};

static const target_t trionic_8_x = {
    instTrionic8,
    "Trionic 8",
    "MC68377, CPU32X",
    typeCPU32,
    nullptr,
    2,
    {
        { opFlash, 0x00000000, 0x00100000 }, // Flash     ( 1M )
        // { opSRAM,  0x00200000, 0x00008000 }, // SRAM      ( 32k )
        { opSRAM,  0x00100000, 0x00001800 }, // DPTRAM    ( 6k )
    }
};

static const target_t trionic_8_x_mcp = {
    instTrionic8mcp,
    "Trionic 8 MCP",
    "MC68F375, CPU32",
    typeCPU32,
    nullptr,
    3,
    {
        { opFlash, 0x00000000, 0x00040100 }, // Flash     ( 256k + 256b shadow )
        { opSRAM,  0x00200000, 0x00002000 }, // SRAM      ( 8k )
        { opSRAM,  0x00100000, 0x00001800 }, // DPTRAM    ( 6k )
    }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Target list

static const target_t *targets[] = {
    &trionic_5_2,       // Trionic 5.2
    &trionic_5_5,       // Trionic 5.5
    &trionic_5_7,       // Trionic 5.5 with 512k of flash
    &trionic_7_7,       // Trionic 7.7
    &trionic_8_x,       // Trionic 8
    &trionic_8_x_mcp,   // Trionic 8 MCP / Copro
    &s60ACC,            // A random ACC unit from Volvo S60 (CPU32)
};

#endif

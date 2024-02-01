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

protected:
    bdmstuff & core;

public:
    explicit iTarget( bdmstuff & p )
        : core(p) { }
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
// Trionic 68k family

// Instantiators
iTarget *instTrionic5( bdmstuff & );
iTarget *instTrionic7( bdmstuff & );

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


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Target list

static const target_t *targets[] = {
    &trionic_5_2,       // Trionic 5.2
    &trionic_5_5,       // Trionic 5.5
    &trionic_5_7,       // Trionic 5.5 with 512k of flash
    &trionic_7_7        // Trionic 7.7
};

#endif

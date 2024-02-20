#ifndef __CPU32_CMFI_H__
#define __CPU32_CMFI_H__

typedef struct {
    uint8_t SCLKR; // ( << 11 ) 0 - 7
    uint8_t CLKPE; // ( <<  8 ) 0 - 3
    uint8_t CLKPM; // ( <<  0 ) 0 - 127
} ctl1_t;

typedef struct {
    uint8_t NVR;   // ( << 11 ) 0 - 1
    uint8_t PAWS;  // ( <<  8 ) 0 - 7
    uint8_t GDB;   // ( <<  5 ) 0 - 1
} tst_t;

enum cpu32_cmfi_ver : uint32_t {
    enCPU32_CMFI_V50     = 0,  // CMFI version 5.0
    enCPU32_CMFI_V51     = 1,  // CMFI version 5.1 ( 5.0TTO )
    enCPU32_CMFI_V60     = 2,  // CMFI version 6.0
    enCPU32_CMFI_V61     = 3,  // CMFI version 6.1
    enCPU32_CMFI_MAX     = 4
};

static const double SCLKR_lut[] = {
    1.0,
    1.0,
    3.0/2.0,
    2.0,
    3.0
};

typedef struct {
    const uint32_t time;     // Pulse time in 10 / us
    const uint32_t pulses;   // Number of pulses
    const tst_t    testdata; // NVR, GDB and PAWS
    const bool     doMargin; // Perform margin read after pulse ( For write, this is ignored )
} cpu32_cmfi_op_t;

// Same as the official driver
#define CPU32_CMFI_MAXOP    ( 9 )

typedef struct {
    const uint32_t        maxErasePulses;
    const cpu32_cmfi_op_t erase[ CPU32_CMFI_MAXOP ];

    const uint32_t        maxWritePulses;
    const cpu32_cmfi_op_t write[ CPU32_CMFI_MAXOP ];
} cpu32_cmfi_seq_t;



//////////////////////////////////////////////////////////////
// CMFI version 5.0
static const cpu32_cmfi_seq_t CPU32_CMFI_V50 = {
    // erase
    1,       // Max erase pulses
    {
        // time 10/us      pulses  { NVR, PAWS, GDB }    Perform margin read
        { 1000 * 10000   , 65535 , { 0  , 0   , 0   } ,  true   } // 0
    },

    // write
    900,     // Max write pulses
    {
        // Isn't this value.. weird?
        // time 10/us      pulses  { NVR, PAWS, GDB }    Perform margin read
        { 256            , 65535 , { 0  , 0   , 0   } ,  true   } // 0
    }
};

static const cpu32_cmfi_seq_t CPU32_CMFI_V51 = {
    // erase
    1,       // Max erase pulses
    {
        // time 10/us      pulses  { NVR, PAWS, GDB }    Perform margin read
        { 10000 * 1000   , 65535 , { 0  , 0   , 0   } ,  true   } // 0
    },

    // write
    10000,   // Max write pulses
    {
        // time 10/us      pulses  { NVR, PAWS, GDB }    Perform margin read
        { 500            , 65535 , { 0  , 0   , 0   } ,  true   } // 0
    }
};


//////////////////////////////////////////////////////////////
// CMFI version 6.0
static const cpu32_cmfi_seq_t CPU32_CMFI_V60 = {
    // erase
    1000,    // Max erase pulses
    {
        // time 10/us      pulses  { NVR, PAWS, GDB }    Perform margin read
        { 10000 * 100    , 1     , { 1  , 4   , 0   } ,  false  }, // 0
        { 10000 * 100    , 1     , { 1  , 5   , 0   } ,  false  }, // 1
        { 10000 * 100    , 1     , { 1  , 6   , 0   } ,  false  }, // 2
        { 10000 * 100    , 1     , { 1  , 7   , 0   } ,  false  }, // 3
        { 10000 * 100    , 1     , { 0  , 4   , 0   } ,  false  }, // 4
        { 10000 * 100    , 1     , { 0  , 5   , 0   } ,  false  }, // 5
        { 10000 * 100    , 1     , { 0  , 6   , 0   } ,  false  }, // 6
        { 10000 * 100    , 65535 , { 0  , 7   , 0   } ,  true   }, // 7
    },

    // write
    48000,   // Max write pulses
    {
        // time 10/us      pulses  { NVR, PAWS, GDB }    Perform margin read
        { 2560           , 4     , { 1  , 4   , 1   } ,  true   }, // 0
        { 2560           , 4     , { 1  , 5   , 1   } ,  true   }, // 1
        { 2560           , 4     , { 1  , 6   , 1   } ,  true   }, // 2
        { 2560           , 4     , { 1  , 7   , 1   } ,  true   }, // 3
        { 500            , 20    , { 0  , 4   , 1   } ,  true   }, // 4
        { 500            , 20    , { 0  , 5   , 1   } ,  true   }, // 5
        { 500            , 20    , { 0  , 6   , 1   } ,  true   }, // 6
        { 500            , 65535 , { 0  , 7   , 1   } ,  true   }, // 7
    }
};

//////////////////////////////////////////////////////////////
// CMFI version 6.1
static const cpu32_cmfi_seq_t CPU32_CMFI_V61 = {
    // erase
    1000,    // Max erase pulses
    {
        // time 10/us      pulses  { NVR, PAWS, GDB }    Perform margin read
        { 10000 * 100    , 1     , { 1  , 4   , 0   } ,  false  }, // 0
        { 10000 * 100    , 1     , { 1  , 5   , 0   } ,  false  }, // 1
        { 10000 * 100    , 1     , { 1  , 6   , 0   } ,  false  }, // 2
        { 10000 * 100    , 1     , { 1  , 7   , 0   } ,  false  }, // 3
        { 10000 * 100    , 1     , { 0  , 4   , 0   } ,  false  }, // 4
        { 10000 * 100    , 1     , { 0  , 5   , 0   } ,  false  }, // 5
        { 10000 * 100    , 1     , { 0  , 6   , 0   } ,  false  }, // 6
        { 10000 * 100    , 65535 , { 0  , 7   , 0   } ,  true   }, // 7
    },

    // write
    24000,   // Max write pulses
    {
        // time 10/us      pulses  { NVR, PAWS, GDB }    Perform margin read
        { 2560           , 4     , { 1  , 4   , 1   } ,  true   }, // 0
        { 2560           , 4     , { 1  , 5   , 1   } ,  true   }, // 1
        { 2560           , 4     , { 1  , 6   , 1   } ,  true   }, // 2
        { 2560           , 4     , { 1  , 7   , 1   } ,  true   }, // 3
        { 500            , 20    , { 0  , 4   , 1   } ,  true   }, // 4
        { 500            , 20    , { 0  , 5   , 1   } ,  true   }, // 5
        { 500            , 20    , { 0  , 6   , 1   } ,  true   }, // 6
        { 500            , 1     , { 0  , 7   , 1   } ,  true   }, // 7
        { 1000           , 65535 , { 0  , 7   , 1   } ,  true   }, // 8
    }
};

#endif

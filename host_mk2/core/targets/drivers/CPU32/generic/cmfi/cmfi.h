#ifndef __CMFI_H__
#define __CMFI_H__

#include <stdio.h>
#include <stdint.h>

enum enOperation {
    OP_WRITE         = 1, // Write
    OP_ERASE         = 2, // Erase according to mask
    OP_BULK          = 3, // Erase everything
    OP_INIT          = 4  // Init driver. Set up stack etc. Must be run before anything else
};

enum enStatus {
    STATUS_UNK       =  0,
    STATUS_OK        =  1,
    STATUS_NODATA    = 10,  // Saw length of 0
    STATUS_ALIGN     = 11,  // Address and length must be in multiples of 64

    STATUS_NOPULSE   = 20,  // maxProgramPulses was 0
    STATUS_MAXPULSE  = 21,  // Reached pulse limit before the operation was complete
};

// Enabled if SES is x
// < 1 > SES must be one for this to function
// < 0 > SES must be zero for this to function
// < ? > Not specified by Motorola


// - - CMFIMCR - -
#define MCR_STOP    (   1   << 15 ) // STOP      15 - < ? > Stop mode enabled of 1
#define MCR_PROTECT (   1   << 14 ) // PROTECT   14 - < 0 > Protect CMFI blocks if 1
#define MCR_SIE     (   1   << 13 ) // SIE       13 - < --> Shadow Information Enable if 1    ( It's protected if SES == 1 && PE == 0 )
#define MCR_BOOT    (   1   << 12 ) // _BOOT_    12 - < ? > Respond to boot after reset if 0
#define MCR_LOCK    (   1   << 11 ) // _LOCK_    11 - < ? > Disable writes to most registers if 0
#define MCR_EMUL    (   1   << 10 ) // EMUL      10 - < ? > External eeprom emulation if 1
#define MCR_ASPC    (   3   <<  8 ) // ASPC    9: 8 - < ? > Data space bits
#define MCR_WAIT    (   3   <<  6 ) // WAIT    7: 6 - < ? > Insert additional wait states

// - - CMFITST - -
#define TST_NVR     (   1   << 11 ) // NVR       11 - < ? > Negative Voltage Range. 0 == High range. 1 == Regular range.  ( GDB must be 1 and HVS 0 for it to function )
#define TST_PAWS    (   7   <<  8 ) // PAWS   10: 8 - < ? > Program Amplitude/Width moulation Select.                     ( Just use what Motorola spec'd              )
#define TST_STE     (   1   <<  6 ) // STE        6 - < ? > Motorola testing. Leave at 0
#define TST_GDB     (   1   <<  5 ) // GDB        5 - < 0 > Gate/Drain Bias select 0 pos, 1 neg                           ( Again, do what Motorola spec'd             )

// CTL bits 
// - - CMFICTL1 - -
// Note: SCLKR, CLKPE and CLKPM are _NOT_ affected by SES if PAWS != 0
#define CTL_HVS     (   1UL << 31 ) // HVS       15 - < ? > High Voltage Status.          - Indicates if H/V is on
#define CTL_SCLKR   (   7UL << 27 ) // SCLKR  13:11 - <0/x> System Clock Range
#define CTL_CLKPE   (   3UL << 24 ) // CLKPE   9: 8 - <0/x> Clock Period Exponent
#define CTL_CLKPM   ( 127UL << 16 ) // CLKPM   6: 0 - <0/x> Clock Period Multiple select
// - - CMFICTL2 - -
#define CTL_BLOCK   ( 255UL <<  8 ) // BLOCK  15: 8 - < 0 > Block select
#define CTL_PEEM    (   1UL <<  5 ) // PEEM       5 - < x > Program Erase Enable Monitor. - No VPP if 0
#define CTL_B0EM    (   1UL <<  4 ) // B0EM       4 - < x > Block 0 Enable Monitor.       - Sector/Block 0 is in RO mode if 0
#define CTL_PE      (   1UL <<  2 ) // PE         2 - < 0 > Program Erase select.         - 0 = write, 1 = erase
#define CTL_SES     (   1UL <<  1 ) // SES        1 - < - > Start-end program or erase Sequence
#define CTL_EHV     (   1UL <<  0 ) // EHV        0 - < 1 >



#define MAX_OP  ( 9 )

// You can find these, in order, starting from offset 4 inside the loader blob.
// Offset and size are in DECIMAL

// Offset 4 ( 16 bytes )
typedef struct {
    const    uint32_t         base;     // Where, in memory space, is the array located
    volatile uint16_t * const CMFIMCR;  // Pointing at CMFIMCR
    volatile uint16_t * const CMFITST;  // Pointing at CMFITST
    volatile uint32_t * const CMFICTL;  // Pointing at CMFICTL1 AND CMFICTL2
    volatile uint8_t  * const SWSR;
} regData_t;

// Offset 24 ( 84 bytes )
typedef struct __attribute__((packed)) {
    const uint32_t ctlProg         [ MAX_OP ];
    const uint16_t pawsProgData    [ MAX_OP ];
    const uint16_t pawsProgPulses  [ MAX_OP ];
    const uint8_t  pawsProgMode    [ MAX_OP ];
    const uint8_t  reserved;
    const uint16_t maxProgramPulses;
} writeData_t;

// Offset 108 ( 84 bytes )
typedef struct __attribute__((packed)) {
    const uint32_t ctlErase        [ MAX_OP ];
    const uint16_t pawsEraseData   [ MAX_OP ];
    const uint16_t pawsErasePulses [ MAX_OP ];
    const uint8_t  pawsEraseMode   [ MAX_OP ];
    const uint8_t  reserved;
    const uint16_t maxErasePulses;
} eraseData_t;

#endif

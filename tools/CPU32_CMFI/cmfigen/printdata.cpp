#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdarg>
#include <cstdlib>

#include "../../../host_mk2/core/targets/utils/cpu32/cmfi_cpu32.h"

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

// Each blob should be exactly 200 bytes

static const char *SCLKR_text[] = {
    "( ? )  ",
    "( 1 )  ",
    "( 3/2 )",
    "( 2 )  ",
    "( 3 )  "
};

static const double SCLKR_lut[] = {
    1.0,
    1.0,
    3.0/2.0,
    2.0,
    3.0
};


void printErase( const uint8_t *buf, uint32_t freq ) {
    ctl1_t   ctls[ CPU32_CMFI_MAXOP ] = { 0 };
    tst_t    tsts[ CPU32_CMFI_MAXOP ] = { 0 };
    uint16_t pcnt[ CPU32_CMFI_MAXOP ] = { 0 };
    uint8_t  mode[ CPU32_CMFI_MAXOP ] = { 0 };
    uint16_t MAXPL = buf[ 86 ] << 8 | buf[ 87 ];

    buf += 4; // Skip SYNCR and reserved

    if ( freq < 8000000 || freq > 33000000 ) {
        printf("Unsupported frequency %.3f\n", freq / 1000000.0 );
        return;
    }

    printf("\n- - - F O R M A T - - -\n");
    printf("Max pulses: %u\n", MAXPL);

    for ( size_t i = 0; i < CPU32_CMFI_MAXOP; i++ ) {
        uint16_t CTL1 = buf[0] << 8 | buf[1];
        uint16_t CTL2 = buf[2] << 8 | buf[3];
        buf += 4;

        // 0979
        if ( CTL1 != 0xffff && CTL1 & 0xC480 )
            printf("Found unexpected bits in CTL1! ( %04x )\n", CTL1);
        if ( CTL2 != 0xffff && CTL2 & 0x00FF )
            printf("Found unexpected bits in CTL2!\n");
        
        ctls[ i ].SCLKR = (CTL1 >> 11) & 7;
        ctls[ i ].CLKPE = (CTL1 >>  8) & 3;
        ctls[ i ].CLKPM =  CTL1        & 127;
    }

    for ( size_t i = 0; i < CPU32_CMFI_MAXOP; i++ ) {
        uint16_t TST = buf[0] << 8 | buf[1];
        buf += 2;

        if ( TST != 0xffff && TST & 0xF0DF )
            printf("Found unexpected bits in TST!\n");
        
        tsts[ i ].NVR  = (TST >> 11) & 1;
        tsts[ i ].PAWS = (TST >>  8) & 7;
        tsts[ i ].GDB  = (TST >>  5) & 1;
    }

    for ( size_t i = 0; i < CPU32_CMFI_MAXOP; i++ ) {
        pcnt[ i ] = buf[0] << 8 | buf[1];
        buf += 2;
    }

    for ( size_t i = 0; i < CPU32_CMFI_MAXOP; i++ ) {
        uint8_t tm = *buf++; 
        mode[ i ] = tm;

        if ( tm != 0 && tm != 0x80 )
            printf("Unknown mode flag: %02x ( %u )\n", tm, (uint16_t)i);
    }

    printf("\nStep       SCLKR       CLKPE       CLKPM         NVR       PAWS       GDB       MARGIN       PULSE       TIME\n");

    for ( size_t i = 0; i < CPU32_CMFI_MAXOP; i++ ) {
        if ( ctls[ i ].SCLKR == 0 || ctls[ i ].SCLKR > 4 ) {
            printf("- - Unsup --\n");
            continue;
        }
        printf("%1u", (uint16_t)i);
        printf("          %1u %s", ctls[ i ].SCLKR, SCLKR_text[ ctls[ i ].SCLKR ]);
        printf("   %1u", ctls[ i ].CLKPE);
        printf("           %3u ( %3u )", ctls[ i ].CLKPM, ctls[ i ].CLKPM + 1);

        printf("   %1u", tsts[ i ].NVR);
        printf("         %1u", tsts[ i ].PAWS);
        printf("          %1u", tsts[ i ].GDB);

        // Retrieve pulse time by some... something!
        uint32_t fac = 1 << ( ctls[ i ].CLKPE + 5 + 10 );
        double cmfiFreq = freq / SCLKR_lut[ ctls[ i ].SCLKR ];
        double timeMs = (((1000.0 * fac) / (cmfiFreq / 1000000.0)) * (ctls[ i ].CLKPM + 1)) / 1000000.0;
        printf("         %s", mode[ i ] == 0x80 ? "No " : "Yes");
        printf("          %5u", pcnt[ i ]);
        printf("       %.03f ms\n", timeMs);

        if ( pcnt[ i ] == 65535 )
            break;
    }
}



// Write
// TST  mask - 0xF0DF   ( Clear NVR, PAWS and GDB )
// CTL1 mask - 0xC480   ( Clear SCLKR, CLKPE and CLKPM  )
// CTL2 mask - 0x00FB   ( Clear BLOCK, negate PE )
// - or -
// CTL2 mask - 0xFFFF   ( Leave as is )
void printWrite( const uint8_t *buf, uint32_t freq ) {
    ctl1_t   ctls[ CPU32_CMFI_MAXOP ] = { 0 };
    tst_t    tsts[ CPU32_CMFI_MAXOP ] = { 0 };
    uint16_t pcnt[ CPU32_CMFI_MAXOP ] = { 0 };
    uint16_t MAXPL = buf[ 86 ] << 8 | buf[ 87 ];

    buf += 4; // Skip SYNCR and reserved

    if ( freq < 8000000 || freq > 33000000 ) {
        printf("Unsupported frequency %.3f\n", freq / 1000000.0 );
        return;
    }

    printf("\n- - - W R I T E - - -\n");
    printf("Max pulses: %u\n", MAXPL);

    for ( size_t i = 0; i < CPU32_CMFI_MAXOP; i++ ) {
        uint16_t CTL1 = buf[0] << 8 | buf[1];
        uint16_t CTL2 = buf[2] << 8 | buf[3];
        buf += 4;

        // 0979
        if ( CTL1 != 0xffff && CTL1 & 0xC480 )
            printf("Found unexpected bits in CTL1! ( %04x )\n", CTL1);
        if ( CTL2 != 0xffff && CTL2 & 0x00FF )
            printf("Found unexpected bits in CTL2!\n");
        
        ctls[ i ].SCLKR = (CTL1 >> 11) & 7;
        ctls[ i ].CLKPE = (CTL1 >>  8) & 3;
        ctls[ i ].CLKPM =  CTL1        & 127;
    }

    for ( size_t i = 0; i < CPU32_CMFI_MAXOP; i++ ) {
        uint16_t TST = buf[0] << 8 | buf[1];
        buf += 2;

        if ( TST != 0xffff && TST & 0xF0DF )
            printf("Found unexpected bits in TST!\n");
        
        tsts[ i ].NVR  = (TST >> 11) & 1;
        tsts[ i ].PAWS = (TST >>  8) & 7;
        tsts[ i ].GDB  = (TST >>  5) & 1;
    }

    for ( size_t i = 0; i < CPU32_CMFI_MAXOP; i++ ) {
        pcnt[ i ] = buf[0] << 8 | buf[1];
        buf += 2;
    }

    for ( size_t i = 0; i < CPU32_CMFI_MAXOP; i++ ) {
        uint8_t tm = *buf++; 

        if ( tm != 0 )
            printf("Unknown mode flag: %02x ( %u )\n", tm, (uint16_t)i);
    }

    printf("\nStep       SCLKR       CLKPE       CLKPM         NVR       PAWS       GDB       PULSE       TIME\n");

    for ( size_t i = 0; i < CPU32_CMFI_MAXOP; i++ ) {
        if ( ctls[ i ].SCLKR == 0 || ctls[ i ].SCLKR > 4 ) {
            printf("- - Unsup --\n");
            continue;
        }
        printf("%1u", (uint16_t)i);
        printf("          %1u %s", ctls[ i ].SCLKR, SCLKR_text[ ctls[ i ].SCLKR ]);
        printf("   %1u", ctls[ i ].CLKPE);
        printf("           %3u ( %3u )", ctls[ i ].CLKPM, ctls[ i ].CLKPM + 1);

        printf("   %1u", tsts[ i ].NVR);
        printf("         %1u", tsts[ i ].PAWS);
        printf("          %1u", tsts[ i ].GDB);

        // Retrieve pulse time by some... something!
        uint32_t fac = 1 << ( ctls[ i ].CLKPE + 5 );
        double cmfiFreq = freq / SCLKR_lut[ ctls[ i ].SCLKR ];
        double timeUs = (((1000.0 * fac) / (cmfiFreq / 1000000.0)) * (ctls[ i ].CLKPM + 1)) / 1000.0;
        printf("         %5u", pcnt[ i ]);
        printf("       %.03f us\n", timeUs);

        if ( pcnt[ i ] == 65535 )
            break;
    }
}

void printData( const uint8_t *buf, uint32_t freq ) {
    printWrite( &buf[8]     , freq );
    printErase( &buf[8 + 88], freq );
}


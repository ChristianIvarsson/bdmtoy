#ifndef __CMFI_CPU32_H__
#define __CMFI_CPU32_H__

#include <cstdio>
#include <cstdint>
#include <cmath>

#include "../../../bdmstuff.h"

#include "cmfi_algos.h"

// CPU32 generic CMFI
class CPU32_gencmfi {

    bdmstuff & core;

    // This needs some optimising...
    static bool genOpData(uint8_t               *buf,
                          const cpu32_cmfi_op_t *op,
                          uint32_t               nPulses,
                          double                 cmfiFreq,
                          uint32_t               SCLKR,
                          bool                   erase) {

        uint32_t facOffs  = erase ? 15 : 5;
        uint8_t *ctrlBuf  = buf;
        uint8_t *pawsBuf  = &ctrlBuf [ 36 ];
        uint8_t *pulseBuf = &pawsBuf [ 18 ];
        uint8_t *modeBuf  = &pulseBuf[ 18 ];
        memset( ctrlBuf, 0xff, 72 ); // Default CMFI data to FF
        memset( modeBuf, 0x00, 10 ); // Default mode data to 00

        modeBuf[ 10 ] = (nPulses >> 8);
        modeBuf[ 11 ] =  nPulses;

        struct {
            uint32_t idx;
            uint32_t CLKPM;
            double   match;
        } cmpArray[ 5 ];

        for ( uint32_t i = 0; i < CPU32_CMFI_MAXOP; i++ ) {

            double wantedUs = op[ i ].time / 10.0;

            // End of sequence
            if ( wantedUs == 0 ) {
                if ( i > 0 )
                    break;
                return false;
            }

            // Go over every CLOCKPE and find the nearest match
            for ( uint32_t pe = 0; pe < 4; pe++ ) {

                // Default to hundred seconds of delta
                double lowest = 10000 * 10000;
                uint32_t CLKPM = 512;
                uint32_t fac = 1 << (pe + facOffs);

                // Go over every factor and find the nearest for this CLKPE
                for ( uint32_t pm = 0; pm < 128; pm++ ) {
                    double pTime = (fac * (pm + 1)) / (cmfiFreq / 1000000.0);
                    double delta = fabs(pTime - wantedUs);
                    if ( delta < lowest ) {
                        CLKPM = pm;
                        lowest = delta;
                    }
                }

                cmpArray[ pe ].idx = pe;
                cmpArray[ pe ].match = lowest;
                cmpArray[ pe ].CLKPM = CLKPM;
            }

            // Sort results
            bool swapped = true;
            while ( swapped ) {
                swapped = false;
                for ( size_t pe = 0; pe < 3; pe++ ) {
                    if ( cmpArray[ pe + 1 ].match < cmpArray[ pe ].match ) {
                        cmpArray[ 5 ] = cmpArray[ pe + 1 ];
                        cmpArray[ pe + 1 ] = cmpArray[ pe ];
                        cmpArray[ pe ] = cmpArray[ 5 ];
                        swapped = true;
                    }
                }
            }

            // Definitely WAY off the mark!
            if ( cmpArray[ 0 ].CLKPM == 512 )
                return false;

            uint32_t CTL = SCLKR << 11 | cmpArray[ 0 ].idx << 8 | cmpArray[ 0 ].CLKPM;
            CTL <<= 16;

            *ctrlBuf++ = CTL >> 24;
            *ctrlBuf++ = CTL >> 16;
            *ctrlBuf++ = CTL >>  8;
            *ctrlBuf++ = CTL;

            uint32_t TST = op[ i ].testdata.NVR  << 11 |
                           op[ i ].testdata.PAWS <<  8 |
                           op[ i ].testdata.GDB  <<  5;
            *pawsBuf++ = TST >> 8;
            *pawsBuf++ = TST;

            *pulseBuf++ = op[ i ].pulses >> 8;
            *pulseBuf++ = op[ i ].pulses;

            modeBuf[ i ] = op[ i ].doMargin ? 0x00 : 0x80;
        }

        return true;
    };

public:
    explicit CPU32_gencmfi( bdmstuff & m )
        : core( m ) {}

    bool generate( uint8_t *buf, cpu32_cmfi_ver ver, uint32_t freq ) {

        // Motorola specifically mentions < 8 as unstable due to a known design flaw
        // Officially, they only go to 33 MHz and no data was given for higher frequencies.
        // At a first glance, it seems they want CMFI to sit between 8 and 12 MHz
        if ( freq < 8000000 || freq > 33000000 ) {
            core.castMessage("Unsupported frequency %.3f", freq / 1000000.0);
            return false;
        }

        if ( ver >= enCPU32_CMFI_MAX ) {
            core.castMessage("Unsupported flash version");
            return false;
        }

        const cpu32_cmfi_seq_t *seq = CMFI_Data[ ver ];
        uint32_t SCLKR;

        // First, figure out divider
        // SCLKR 1   8 - 12 MHz    ( 1 )
        // SCLKR 2  12 - 18 MHz    ( 3 / 2 )
        // SCLKR 3  18 - 24 MHz    ( 2 )
        // SCLKR 4  24 - 33 MHz    ( 3 )
        if ( freq >= 24000000 )
            SCLKR = 4;
        else if ( freq >= 18000000 )
            SCLKR = 3;
        else if ( freq >= 12000000 )
            SCLKR = 2;
        else if ( freq >= 8000000 )
            SCLKR = 1;
        else {
            core.castMessage("You should not see this");
            return false;
        }

        // Convert CPU frequency down to CMFI frequency
        double cmfiFreq = freq / CPU32_CMFI_SCLKR_lut[ SCLKR ];

        ////////////////////////////////
        // Write params

#ifdef CMFI_COMPARE
        // Retain old SYNCR and padding
        buf += 4;
        if ( !genOpData( buf, seq->write, seq->maxWritePulses, cmfiFreq, SCLKR, false) ||
             !genOpData( &buf[88], seq->erase, seq->maxErasePulses, cmfiFreq, SCLKR, true) ) {
            core.castMessage("Unable to generate CMFI data");
            return false;
        }
#else
        core.castMessage("Info: CMFI at %.3f MHz ( SCLKR %u )", cmfiFreq / 1000000.0, SCLKR);
        if ( !genOpData( buf, seq->write, seq->maxWritePulses, cmfiFreq, SCLKR, false) ||
             !genOpData( &buf[84], seq->erase, seq->maxErasePulses, cmfiFreq, SCLKR, true) ) {
            core.castMessage("Unable to generate CMFI data");
            return false;
        }
#endif
        return true;
    }

};

#endif

#ifndef __CMFI_CPU32_H__
#define __CMFI_CPU32_H__

#include <cstdio>
#include <cstdint>
#include <cmath>

#include "../../../../bdmstuff.h"
#include "../../../../requests_cpu32.h"

#include "cpu32_cmfi_algos.h"

// Matching driver
#include "../../../drivers/CPU32/generic/cpu32_cmfi.h"

// Used to print register contents while debugging this thing
#include "../../../target_helper.h"


typedef struct {
    const char     *const pName;
    const cpu32_cmfi_ver  version;
    const uint32_t        count;
    const uint32_t *const partitions;
} CPU32_cmfiPart_t;

// This list is taking driver behaviour into account. Shadow is not actually at offset 40000 - 400ff but at 0 - ff
static constexpr const uint32_t CPU32_cmfiPartitions[] = {
    0x008000, 0x010000, 0x018000, 0x020000,
    0x028000, 0x030000, 0x038000, 0x040000,
    0x040100
};

static constexpr const CPU32_cmfiPart_t CPU32_cmfiParts[] = {
    { "CMFI version 5.0"               , enCPU32_CMFI_V50,   9, CPU32_cmfiPartitions },
    { "CMFI version 5.1 aka 5.0TTO"    , enCPU32_CMFI_V51,   9, CPU32_cmfiPartitions },
    { "CMFI version 6.0"               , enCPU32_CMFI_V60,   9, CPU32_cmfiPartitions },
    { "CMFI version 6.1"               , enCPU32_CMFI_V61,   9, CPU32_cmfiPartitions }
};

static constexpr const double CPU32_CMFI_SCLKR_lut[] = {
    1.0,
    1.0,
    3.0/2.0,
    2.0,
    3.0
};

// CPU32 generic CMFI
class CPU32_gencmfi : public virtual CPU32_requests {

    uint32_t  driverBase;
    uint32_t  cmfiBase;
    uint32_t  bufferBase;
    const CPU32_cmfiPart_t *flashPart;
    bool driverInited;

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

    bool getStatus() {
        uint16_t flTemp[2];
        uint32_t status;
        if ( core.queue.send( readDataRegister(0) ) == false ||
             core.getData(flTemp, TAP_DO_READREGISTER, 4, 0) == false ) {
            core.castMessage("Error: Unable to retrieve driver status");
            return false;
        }

        status = *(uint32_t *)flTemp;

        if ( status != 1 ) {
            switch ( status ) {
            case 10:
                core.castMessage("Error: Driver flagged data length error ( no data )");
                break;
            case 11:
                core.castMessage("Error: Driver flagged address or length alignment error");
                break;
            case 20:
                core.castMessage("Error: Driver flagged no available H/V pulses error");
                break;
            case 21:
                core.castMessage("Error: Driver flagged pulse limits exceeded error");
                break;
            default:
                core.castMessage("Error: Driver flagged unknown code ( %04x )", status);
            }

            return false;
        }

        return true;
    }

    bool demand(uint32_t what) {

        bool retVal;
        uint16_t status;

        core.queue  = writeDataRegister(0, what);
        core.queue += writeSystemRegister(0, driverBase);
        core.queue += targetStart();

        if ( !core.queue.send() ) {
            core.castMessage("Error: Unable to configure and/or start driver");
            return false;
        }

        do {
            sleep_ms( 50 );
            retVal = core.getStatus(&status);
        } while ( retVal && status == RET_TARGETRUNNING );

        if ( status != RET_TARGETSTOPPED ) {
            core.castMessage("Error: Could not stop target");
            return false;
        }

        return getStatus();
    }

public:

    explicit CPU32_gencmfi( bdmstuff & p )
        : requests(p), CPU32_requests(p) {
        driverBase = 0;
        cmfiBase = 0;
        bufferBase = 0;
        flashPart = nullptr;
        driverInited = false;
    }

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

    bool setShadow( bool state ) {
        uint16_t shadowSet;

        if ( !core.queue.send(readMemory( 0xFFF800, sizeWord )) ||
             !core.getData( &shadowSet, TAP_DO_READMEMORY, sizeWord, 0 )) {
            core.castMessage("Error: Unable to retrieve current shaddow settings");
            return false;
        }

        // Return if already in the correct mode
        if ( ( state && (shadowSet & 0x2000) != 0) ||
             (!state && (shadowSet & 0x2000) == 0) )
            return true;

        if ( state )
            shadowSet |= 0x2000;
        else
            shadowSet &= 0xDFFF;

        if ( !core.queue.send(writeMemory( 0xFFF800, shadowSet, sizeWord )) ) {
            core.castMessage("Error: Unable to set new shaddow settings");
            return false;
        }

        return true;
    }

    bool upload(const memory_t        *region,
                uint32_t               destination,
                uint32_t               buffer,
                const cpu32_cmfi_ver & version)  {

         core.castMessage("Info: Uploading cmfi driver..");

         driverInited = false;
         driverBase = destination;
         bufferBase = buffer;

        if ( version >= enCPU32_CMFI_MAX )
        {
            core.castMessage("Error: Invalid CMFI type");
            return false;
        }

        flashPart = &CPU32_cmfiParts[ version ];

        if ( fillDataBE4(destination, CPU32_cmfi, sizeof(CPU32_cmfi)) == false ) {
            core.castMessage("Error: Unable to upload driver");
            return false;
        }

        if ( !demand(4) )
            return false;

        driverInited = true;

        return true;
    }

    bool sectorErase(uint32_t mask) {
        uint16_t status;
        bool retVal;

        core.castMessage("Info: Erasing with mask %02x", mask);

        if ( !driverInited ) {
            core.castMessage("Error: You must upload the flash driver before using this feature");
            return false;
        }

        core.queue  = writeDataRegister(0, 2);
        core.queue += writeDataRegister(1, mask);
        core.queue += writeSystemRegister(0, driverBase);
        core.queue += targetStart();

        if ( !core.queue.send() ) {
            core.castMessage("Error: Unable to configure and/or start erase");
            return false;
        }

        do {
            retVal = core.getStatus(&status);
            sleep_ms(25);
        } while ( retVal && status == RET_TARGETRUNNING );

        if ( status != RET_TARGETSTOPPED ) {
            core.castMessage("Error: Could not stop target");
            return false;
        }

        return getStatus();
    }

    bool bulkErase() {
        uint16_t status;
        bool retVal;

        core.castMessage("Info: Erasing all blocks");

        if ( !driverInited ) {
            core.castMessage("Error: You must upload the flash driver before using this feature");
            return false;
        }

        core.queue  = writeDataRegister(0, 3);
        core.queue += writeSystemRegister(0, driverBase);
        core.queue += targetStart();

        if ( !core.queue.send() ) {
            core.castMessage("Error: Unable to configure and/or start erase");
            return false;
        }

        do {
            retVal = core.getStatus(&status);
            sleep_ms(25);
        } while ( retVal && status == RET_TARGETRUNNING );

        if ( status != RET_TARGETSTOPPED ) {
            core.castMessage("Error: Could not stop target");
            return false;
        }

        return getStatus();
    }

    // Write cmfi
    // Couple of adapter quirks in CPU32 mode that must be known:
    // 1 - It won't adjust buffer size according to remaining length so request must be in multiples of that
    // 2 - There must never be less than 8 bytes per write iteration
    // 3 - Everything must be in multiples of 4
    bool write(uint32_t mask) {
        uint32_t Start = cmfiBase;
        uint32_t maskOffs = 0;
        memory_t memSpec = { opFlash };
        size_t   totLen = 0;

        if ( !driverInited ) {
            core.castMessage("Error: You must upload the flash driver before using this feature");
            return false;
        }

        if ( flashPart == nullptr || flashPart->count == 0 ) {
            core.castMessage("Error: Need valid partition parameters");
            return false;
        }

        if ( flashPart->count > 32 ) {
            core.castMessage("Error: I don't know what to do with this many partitions!");
            return false;
        }

        // Write all
        if ( mask == 0 )
            mask = ~mask;

        // Count number of total bytes to write
        while ( maskOffs < flashPart->count ) {
            if ( ((1 << maskOffs) & mask) == 0 ) {
                maskOffs++;
                continue;
            }
            if ( maskOffs > 0 )
                Start = cmfiBase + flashPart->partitions[ maskOffs - 1 ];
            totLen += ((cmfiBase + flashPart->partitions[ maskOffs++ ]) - Start);
        }

        // Set total len
        core.pagedProgress( true, totLen );

        // Start over and do it for real this time
        Start = cmfiBase;
        maskOffs = 0;

        core.castMessage("Info: Total length %u bytes", totLen);

        // Driver needs to know where the buffer is located
        if ( core.queue.send(writeAddressRegister(1, bufferBase)) == false ) { 
            core.castMessage("Unable to set write start pointer");
            return false;
        }

        while ( maskOffs < flashPart->count ) {

            // Not set, loop over and check the next bit
            if ( ((1 << maskOffs) & mask) == 0 ) {
                maskOffs++;
                continue;
            }


            if ( maskOffs > 0 )
                Start = cmfiBase + flashPart->partitions[ maskOffs - 1 ];

            maskOffs++;

            // Traverse the mask and figure out the largest continuous block
            while ( ((1 << maskOffs) & mask) != 0 && maskOffs < flashPart->count )
                maskOffs++;

            uint32_t Length = (cmfiBase + flashPart->partitions[ maskOffs - 1 ]) - Start;
            uint32_t bufSize = 0x400;
            uint32_t runtBytes = 0;

            // Host performs a couple of checks on buffer requests from the adapter so it needs to be aware of what's up
            memSpec.address = Start;
            memSpec.size = Length;
            core.setRange( &memSpec );


            // Check for odd lengths etc
            if ( Length < bufSize )
                bufSize = Length;
            else if ( (runtBytes = (Length % bufSize)) != 0)
                Length -= runtBytes;

            // Some paranoia
            if ( Length == 0 ) {
                core.castMessage("Error: write() - Internal coding error");
                return false;
            }
            if ( (Length & 7) != 0 ) {
                core.castMessage("Error: Can't write in other than multiples of 8 (%u)", Length);
                return false;
            }

            core.castMessage("Info: Writing %06x - %06x..", Start, Start + Length - 1);

            // Assist function is VERY stupid on CPU32
            core.queue  = writeAddressRegister(0, Start);
            core.queue += writeDataRegister(1, bufSize / 2);
            if ( !core.queue.send() ) {
                core.castMessage("Error: Unable to update buffer size");
                return false;
            }

            if ( !core.queue.send(assistFlash(
                                    Start, Length,
                                    driverBase, bufferBase, bufSize)) ) {
                core.castMessage("Error: Flash failed");
                return false;
            }


            // Swoop up left-over data and send it to the target
            if ( runtBytes != 0 ) {

                if ( (runtBytes & 7) != 0 ) {
                    core.castMessage("Error: Can't write in other than multiples of 8 (%u)", runtBytes);
                    return false;
                }

                core.castMessage("Info: Writing %06x - %06x..", Start + Length, Start + Length + runtBytes - 1);

                if ( core.queue.send(writeDataRegister(1, runtBytes)) == false ) { 
                    core.castMessage("Error: Unable to update buffer size");
                    return false;
                }

                if ( !core.queue.send(assistFlash(
                                        Start + Length, runtBytes,
                                        driverBase, bufferBase, runtBytes)) ) {
                    core.castMessage("Error: Flash failed");
                    return false;
                }
            }
        }

        return true;
    }
};

#endif

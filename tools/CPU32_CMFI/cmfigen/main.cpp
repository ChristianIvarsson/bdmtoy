#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdarg>
#include <cstdlib>
#include <math.h>

#include "cpu32_cmfi.h"

extern void printData( const uint8_t *buf, uint32_t freq );

#define OP_VERS    ( enCPU32_CMFI_V61 )
#define PROC_FREC  ( 245 * 100000 )

static const char *vNAMES[] = {
    "A50_",
    "A51_",
    "A60_",
    "A61_"
};

const cpu32_cmfi_seq_t *CPU32_CMFI_Data[ enCPU32_CMFI_MAX ] = {
    &CPU32_CMFI_V50,   // Version 5.0
    &CPU32_CMFI_V51,   // Version 5.1
    &CPU32_CMFI_V60,   // Version 6.0
    &CPU32_CMFI_V61    // Version 6.1
};

size_t openRead( const char *fName, uint8_t *buf, size_t nBytes ) {
    FILE *fp;
    if ( (fp = fopen( fName, "rb" )) == nullptr ) {
        printf("Error: openRead() - Could not get file handle\n");
        return 0;
    }

    fseek(fp, 0L, SEEK_END);
    long fileSize = ftell( fp );
    rewind(fp);

    if ( fileSize < 0 ) {
        printf("Error: openRead() - Unable to determine file size\n");
        fclose( fp );
        return 0;
    }

    if ( fileSize == 0 ) {
        printf("Error: openRead() - There's no data to read\n");
        fclose( fp );
        return 0;
    }

    // Files could be of another size. Problem for future me
    if ( fileSize != (long)nBytes ) {
        printf("Error: openRead() - File is not of the expected size\n");
        fclose( fp );
        return 0;
    }

    // Read data
    if ( fread( buf, 1, fileSize, fp ) != (size_t)fileSize ) {
        printf("Error: makeHeader() - Unable to read byte from input\n");
        fclose( fp );
        return 0;
    }

    fclose( fp );

    return (size_t)fileSize;
}

bool openVersion( uint8_t *buf, cpu32_cmfi_ver ver, uint32_t freq ) {

    char fileName[ 256 + 1 ] = "array_blobs/gmd_cpu32_cmf_300_";
    size_t sizeUsed = strlen(fileName);

    // Copy string AND terminating zero
    memcpy( &fileName[ sizeUsed ], vNAMES[ (uint32_t)ver ], 5);
    sizeUsed = strlen(fileName);

    snprintf(&fileName[ sizeUsed ], 256 - sizeUsed, "%03u.bin", freq / 100000);

    // printf("Opening %s\n", fileName);

    if ( openRead(fileName, buf, 200) == 0 )
        return false;

    return true;
}

static const char *getName( cpu32_cmfi_ver ver, uint32_t freq ) {

    static char blobName[ 256 + 1 ] = "gmd_cpu32_cmf_300_";
    size_t sizeUsed = 18; // strlen(blobName);

    // Copy string AND terminating zero
    memcpy( &blobName[ sizeUsed ], vNAMES[ (uint32_t)ver ], 5);
    sizeUsed = strlen(blobName);

    snprintf(&blobName[ sizeUsed ], 256 - sizeUsed, "%03u", freq / 100000);

    return blobName;
}

static bool genOpData(uint8_t               *buf,
                      const cpu32_cmfi_op_t *op,
                      uint32_t               nPulses,
                      double                 cmfiFreq,
                      uint32_t               SCLKR,
                      bool                   erase) {

    uint32_t facOffs = erase ? 15 : 5;

    uint8_t *ctrlBuf  = buf;
    uint8_t *pawsBuf  = &ctrlBuf [ 36 ];
    uint8_t *pulseBuf = &pawsBuf [ 18 ];
    uint8_t *modeBuf  = &pulseBuf[ 18 ];
    memset( ctrlBuf, 0xff, 72 ); // Default CMFI data to FF
    memset( modeBuf, 0x00, 10 ); // Default mode data to 00

    typedef struct {
        uint32_t idx;
        uint32_t CLKPM;
        double   match;
    } comp_t;

    modeBuf[ 10 ] = (nPulses >> 8);
    modeBuf[ 11 ] =  nPulses;

    for ( uint32_t i = 0; i < CPU32_CMFI_MAXOP; i++ ) {

        double wantedUs = (double)op[ i ].time;
        comp_t cmpArray[ 5 ];

        // End of sequence
        if ( wantedUs == 0 )
            break;

        // Go over every CLOCKPE and find the nearest match
        for ( uint32_t pe = 0; pe < 4; pe++ ) {

            // Default to ten seconds of delta
            double lowest = 10000 * 10000;
            uint32_t CLKPM = 0;
            uint32_t fac = 1 << (pe + facOffs);

            // Go over every factor and find the nearest for this CLKPE
            for ( uint32_t pm = 0; pm < 128; pm++ ) {
                double pTime = (fac * (pm + 1)) / (cmfiFreq / 10000000.0);
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

        // for ( uint32_t pe = 0; pe < 4; pe++ )
        //     printf("PE %u: %.2f\n", cmpArray[ pe ].idx,  cmpArray[ pe ].match);
        // printf("Picked index %d\n\n", cmpArray[ 0 ].idx);

        // break;
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

    return false;
};

bool generate( uint8_t *buf, cpu32_cmfi_ver ver, uint32_t freq ) {
    const cpu32_cmfi_seq_t *seq;
    uint32_t SCLKR;
    double cmfiFreq;

    // Motorola specifically mentions < 8 as unstable due to a known design flaw
    // Officially, they only go to 33 MHz and no data was given for higher frequencies.
    // At a first glance, it seems they want CMFI to sit between 8 and 12 MHz
    if ( freq < 8000000 || freq > 33000000 ) {
        printf("Unsupported frequency %.3f\n", freq / 1000000.0 );
        return false;
    }

    if ( ver >= enCPU32_CMFI_MAX ) {
        printf("Unsupported flash version\n");
        return false;
    }

    seq = CPU32_CMFI_Data[ ver ];

    // printf("\n- - - G E N E R A T I N G - - -\n");

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
        printf("You should not see this\n");
        return false;
    }

    // Convert CPU frequency down to CMFI frequency
    cmfiFreq = freq / SCLKR_lut[ SCLKR ];

    // printf("CMFI frequency: %.3f MHz ( SCLKR %u )\n\n", cmfiFreq / 1000000.0, SCLKR);

    ////////////////////////////////
    // Write params

    // Remember to comment out when used hot!
    // Retain old SYNCR and padding
    buf += 4;

    genOpData( buf, seq->write, seq->maxWritePulses, cmfiFreq, SCLKR, false);

    genOpData( &buf[88], seq->erase, seq->maxErasePulses, cmfiFreq, SCLKR, true);

    return true;
}

bool compareAlgos( cpu32_cmfi_ver ver, uint32_t freq ) {
    uint8_t bufA[ 512 ];
    uint8_t bufB[ 512 ];
    uint32_t offset = 8;

    if ( openVersion( bufA, ver, freq ) == false )
        return false;

    // Make sure bufferB is anything but the same as bufferA
    for ( uint32_t i = 0; i < sizeof(bufA); i++ ) {
        bufB[ i ] = ~bufA[ i ];
    }

    // Copy SYNCR and padding
    memcpy( &bufB[ offset      ], &bufA[ offset      ], 4 );
    memcpy( &bufB[ offset + 88 ], &bufA[ offset + 88 ], 4 );

    // Skip past header since we're not using any of that
    if ( generate( &bufB[ offset ], ver, freq ) == false )
        return false;

    if ( freq < 8000000 || freq > 33000000 ) {
        printf("Unsupported frequency %.3f\n", freq / 1000000.0 );
        return false;
    }

    for ( size_t i = 0; i < 2; i++ ) {

        // Write data -> erase data
        uint32_t facOffset = (i == 1) ? 15 : 5;

        if ( memcmp(&bufA[offset], &bufB[offset], 4) != 0 ) {
            printf("Mismatched SYNCR!\n");
            return false;
        }

        // Doing bruteforce calc for closest match instead of some clever algo so it's only possible to compare the results of each version
        uint8_t *ctrlBufA = &bufA[offset + 4];
        uint8_t *ctrlBufB = &bufB[offset + 4];

        // Go over every OP and compare logical results
        for ( uint32_t ct = 0; ct < CPU32_CMFI_MAXOP; ct++ ) {
            // SCLKR
            uint32_t SCLKRa = (ctrlBufA[ 0 ] >> 3) & 7;
            uint32_t SCLKRb = (ctrlBufB[ 0 ] >> 3) & 7;
            // CLKPE
            uint32_t CLKPEa = (ctrlBufA[ 0 ] ) & 3;
            uint32_t CLKPEb = (ctrlBufB[ 0 ] ) & 3;
            // CLKPM
            uint32_t CLKPMa = (ctrlBufA[ 1 ] ) & 127;
            uint32_t CLKPMb = (ctrlBufB[ 1 ] ) & 127;

            // CTRL2 isn't normally set from the binary array
            if ( memcmp(&ctrlBufA[ 2 ], &ctrlBufB[ 2 ], 2) != 0 ) {
                printf("Mismatched CTRL data!\n");
                return false;
            }

            ctrlBufA +=4;
            ctrlBufB +=4;

            // Skip this sequence if FF
            if ( ctrlBufA[ -3 ] == 0xff && ctrlBufA[ -2 ] == 0xff &&
                 ctrlBufB[ -3 ] == 0xff && ctrlBufB[ -2 ] == 0xff )
                 continue;

            // Compare verbatim.
            if ( SCLKRa != SCLKRb ) {
                printf("SCLKR mismatch!\n");
                return false;
            }

            if ( SCLKRa == 0 || SCLKRa > 4 ) {
                printf("Unsupported SCLKR value!\n");
                return false;;
            }

            double cmfiFreq = freq / SCLKR_lut[ SCLKRa ];

            // These two must be compared by resulting values instead of used values
            uint32_t facA = 1 << ( CLKPEa + facOffset );
            uint32_t facB = 1 << ( CLKPEb + facOffset );

            double timeA = (facA * (CLKPMa + 1)) / (cmfiFreq / 1000000.0);
            double timeB = (facB * (CLKPMb + 1)) / (cmfiFreq / 1000000.0);

            if ( timeA != timeB ) {
                printf("Pulse time mismatch! ( i: %u, %.2f vs %.2f )\n", ct, timeA, timeB );
                return false;
            }
        }

        // Compare PAWS data
        if ( memcmp(&bufA[offset + 40], &bufB[offset + 40], 18) != 0 ) {
            printf("Mismatched PAWS data!\n");
            return false;
        }

        // Compare pulse data
        if ( memcmp(&bufA[offset + 58], &bufB[offset + 58], 18) != 0 ) {
            printf("Mismatched pulse data!\n");
            return false;
        }

        // Compare paws mode data
        if ( memcmp(&bufA[offset + 76], &bufB[offset + 76], 9) != 0 ) {
            printf("Mismatched PAWS mode data!\n");
            return false;
        }

        // Compare max pulse data
        if ( memcmp(&bufA[offset + 86], &bufB[offset + 86], 2) != 0 ) {
            printf("Mismatched max pulse data!\n");
            return false;
        }

        offset += 88;
    }

    printf("All matched\n");

    return true;
}

void checkAll() {
    for ( uint32_t v = 0; v < enCPU32_CMFI_MAX; v++ )
    for ( uint32_t f = 80; f <= 330; f+=5 ) {
        printf("%s .. ", getName( (cpu32_cmfi_ver)v, f * 100000 ));
        if ( compareAlgos( (cpu32_cmfi_ver)v, f * 100000 ) == false ) {
            printf("Gave up at version index %u ( Freq %.2f MHz )\n", v, f / 10.0);
            return ;
        }
    }
}

int main(int argc, char *argv[]) {

     checkAll();

   
/*
    uint8_t buffer[ 512 ];
    uint32_t freq = PROC_FREC;

    compareAlgos( OP_VERS, freq );

    if ( openVersion( buffer, OP_VERS, freq ) == false )
        return 1;
    printData( buffer, freq );

    // Skip junk in beginning of buffer
    if (!generate( &buffer[8], OP_VERS, freq ))
        return 1;
    printData( buffer, freq );
*/
    return 0;
}

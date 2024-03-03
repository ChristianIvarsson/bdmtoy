//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Trionic targets

#include <cstring>
#include "../bdmstuff.h"
#include "targets.h"
#include "../requests_cpu32.h"
#include "utils/cpu32/utils_cpu32.h"

enum txFamily : uint32_t {
    txGeneric = 0,
    txTrionic5,
    txTrionic7,
    txTrionic8,
    txTrionic8mcp
};

class iTrionic
    : public cpu32_utils, public virtual CPU32_requests, public iTarget {

protected:

    bool writeFlash_mk2( txFamily gen, const target_t *, const memory_t *region ) {

        md5k_t remoteKeys, localKeys;
        flashid_t fID = { 0 };
        parthelper pid;
        crypto::md5 local_md5;
        uint16_t hwTemp[2];
        uint32_t chipCount = (gen == txTrionic5) ? 2 : 1;

        if ( region == nullptr ) {
            core.castMessage("Error: This routine needs to know base address");
            return false;
        }

        if ( region->type != opFlash ) {
            core.castMessage("Error: This routine only knows how to deal with flash");
            return false;
        }

        core.setTimeout( 61 * 1000 );



        // Detect flash type
        uint32_t flashBase = region->address;

        if ( gen == txTrionic5 ) {

            // Avoid enabling H/V until L/V chips has been tested
            if ( !flash.detect_mk2( fID, flashBase, enWidth8 , 2 ) ) {

                core.castMessage("Info: This is trionic 5. Testing H/V..");

                core.queue  = writeMemory( 0xFFFC14, 0x0040, sizeWord );
                core.queue += readMemory ( 0xFFFC16, sizeWord  );

                if ( !core.queue.send() ||
                     !core.getData(hwTemp, TAP_DO_READMEMORY, 2, 0) ||
                     !core.queue.send( writeMemory( 0xFFFC16, hwTemp[0] | 0x0040, sizeWord )) ) {
                    core.castMessage("Error: Unable to enable H/V");
                    return false;
                }

                if ( !flash.detect_mk2( fID, flashBase, enWidth8 , 2 ) )
                    return false;
            }
        } else {
            if ( !flash.detect_mk2( fID, flashBase, enWidth16, 1 ) )
                return false;
        }



        // Get address map of this flash
        const flashpart_t *part = pid.getMap( fID.MID, fID.DID, (gen == txTrionic5) ? enWidth8 : enWidth16);

        if ( part == nullptr ) {
            core.castMessage("Error: Host does not understand this flash");
            return false;
        }

        if ( part->count > 32 ) {
            core.castMessage("Error: Too many partitions");
            return false;
        }



        // Check if a file needs to be mirrored
        if ( gen == txTrionic5 || gen == txTrionic7 ) {

            size_t flashSize = part->partitions[ part->count - 1 ];
            size_t minAllow = (gen == txTrionic5) ? 0x20000 : 0x080000;
            size_t maxAllow = (gen == txTrionic5) ? 0x80000 : 0x100000;

            // Trionic 5 has two of these
            flashSize = (gen == txTrionic5) ? (flashSize * 2) : flashSize;

            if ( flashSize < minAllow || flashSize > maxAllow ) {
                core.castMessage("Error: Not seeing the correct flash size for this ECU");
                return false;
            }

            if ( core.fileSize >= minAllow && core.fileSize < flashSize ) {
                core.castMessage("Info: File is too small. Checking if it can be mirrored..");
                if ( core.mirrorReadFile( flashSize ) == false )
                    return false;
                core.castMessage("Info: It could! Let's go");
            }

            // Core will automatically increment fileSize while mirroring
            if ( core.fileSize != flashSize ) {
                core.castMessage("Error: File size does not match expected size");
                return false;
            }
        }



        core.castMessage("Info: Comparing md5..");

        uint32_t mask = 0;
        if ( !md5.upload( 0x100000, true ) )
            return false;

        for ( size_t i = 0; i < part->count; i++ ) {

            uint32_t start  = (i == 0) ? 0 : (part->partitions[ i - 1 ] * chipCount);
            uint32_t length = (part->partitions[ i ] * chipCount) - start;

            local_md5.hash( &localKeys, &core.buffer[ start ], length );

            // Flash base does not necessarily sit at 0 so this has to be appended after the local file has been hashed
            start += flashBase;

            if ( !md5.hash( &remoteKeys, start, length, true ) )
                return false;

            if ( memcmp( &remoteKeys, &localKeys, sizeof(md5k_t) ) != 0 ) {
                mask |= ( 1 << i );
                core.castMessage("Info: part %2u (%06x - %06x) - different", i, start, start + length - 1);
            } else {
                core.castMessage("Info: part %2u (%06x - %06x) - identical", i, start, start + length - 1);
            }
        }

        core.castMessage("Info: Partition mask %08x", mask);

        if ( mask == 0 ) {
            core.castMessage("Info: Everything is identical");
            return true;
        }

        // Force bulk erase for testing
        // mask = 0;

        if ( !core.swapBuffer( 4, core.fileSize ) )
            return false;

        if ( !flash.upload(part, 0x100400, 0x100000, chipCount) )
            return false;

        if ( !flash.erase(mask) )
            return false;

        return ( flash.write(mask) && core.queue.send( targetReset() ) );
    }

    bool writeSRAM( txFamily gen, const target_t *, const memory_t *region ) {
        memory_t memSpec = { opSRAM };

        core.castMessage("Info: Writing SRAM..");

        if ( region == nullptr ) {
            core.castMessage("Error: Unable to determine destination address");
            return false;
        }

        memSpec.address = region->address;
        memSpec.size    = core.fileSize;

        // This isn't strictly necessary while sending manual buffers
        core.setRange( &memSpec );

        if ( fillDataBE4( memSpec.address, core.buffer, memSpec.size ) == false ) {
            core.castMessage("Error: Unable to upload blob");
            return false;
        }

        return true;
    }

public:
    iTrionic(bdmstuff & p)
        : requests(p), CPU32_requests(p), cpu32_utils(p) { }
    ~iTrionic() { }

    virtual bool read(const target_t *, const memory_t *mem) {
        if ( core.queue.send( assistDump( mem->address, mem->size ) ) == false ) {
            core.castMessage("Info: iTrionic::read - Unable to start dump");
            return false;
        }
        return core.swapDump( 4 );
    }

    virtual bool write( const target_t *target , const memory_t *region ) {
        if ( region->type == opFlash )
            return writeFlash_mk2( txGeneric, target, region );
        else if ( region->type == opSRAM )
            return writeSRAM( txGeneric, target, region );
        return false;
    }
};


class gm_techII
    : public iTrionic {
public:
    gm_techII(bdmstuff &p)
        : requests(p), CPU32_requests(p), iTrionic(p) { }
    ~gm_techII() { }

    bool write( const target_t *target , const memory_t *region ) {
        if ( region->type == opFlash )
            return writeFlash_mk2( txTrionic5, target, region );
        else if ( region->type == opSRAM )
            return writeSRAM( txTrionic5, target, region );
        return false;
    }

    bool init(const target_t *, const memory_t *) {
        TAP_Config_host_t config;
        // uint16_t buf[ 4 ];

        core.castMessage("Info: gm_techII::init");

        config.Type = TAP_IO_BDMOLD;
        config.Frequency = 1500000;
        config.cfgmask.Endian = TAP_BIGENDIAN;

        // Interface config
        core.queue  = setInterface( config );
        core.queue += targetReset();
        core.queue += targetReady();
        core.queue += writeSystemRegister( CPU32_SREG_SFC, 5 );  // Core CPU32 configuration
        core.queue += writeSystemRegister( CPU32_SREG_DFC, 5 );
        core.queue += writeMemory( 0xFFFA21, 0x0000, sizeByte ); // SYPCR - Set watchdog enable to 0
        core.queue += writeMemory( 0xFFFA04, 0x7f00 /*0xCF00*/, sizeWord ); // SYNCR - Set clock bits. Use T5's clock settings
        if ( core.queue.send() == false ) return false;
/*
WE: PE0 / DSACK0 , R/_W_
RP: ??
          CS   CE
flash 0
Flash 1
*/
        ////////////////////////////////////////////////////////////////////
        // Configure chip select and base of all memory mapped, external, devices.

        core.queue  = writeMemory( 0xfffa44, 0x2AFF, sizeWord ); // CSPAR0  - ~
        core.queue += writeMemory( 0xfffa46, 0x00f9, sizeWord ); // CSPAR1  - ~
        core.queue += writeMemory( 0xfffa48, 0x0007, sizeWord ); // CSBARBT - Flash. Base 00_0000, size 1M
        core.queue += writeMemory( 0xfffa4a, 0x78BF, sizeWord ); // CSORBT  - ~
        core.queue += writeMemory( 0xfffa4c, 0x2007, sizeWord ); // CSBAR0  - SRAM.  Base 20_0000, size 1M
        core.queue += writeMemory( 0xFFFA4E, 0x787F, sizeWord ); // CSOR0   - ~
        if ( core.queue.send() == false ) return false;


        core.queue  = writeMemory( 0xFFFB04, 0x1000, sizeWord ); // TPURAM at 10_0000
        if ( core.queue.send() == false ) return false;

        config.Frequency = 1500000;
        return core.queue.send( setInterface( config ) );
    }
};


class trionic_5
    : public iTrionic {
public:
    trionic_5(bdmstuff &p)
        : requests(p), CPU32_requests(p), iTrionic(p) { }
    ~trionic_5() { }

    bool write( const target_t *target , const memory_t *region ) {
        if ( region->type == opFlash )
            return writeFlash_mk2( txTrionic5, target, region );
        else if ( region->type == opSRAM )
            return writeSRAM( txTrionic5, target, region );
        return false;
    }

    bool init(const target_t *, const memory_t *) {
        TAP_Config_host_t config;
        uint16_t buf[ 4 ];

        core.castMessage("Info: trionic_5::init");

        config.Type = TAP_IO_BDMOLD;
        config.Frequency = 1500000;
        config.cfgmask.Endian = TAP_BIGENDIAN;

        // Interface config
        core.queue  = setInterface( config );
        core.queue += targetReset();
        core.queue += targetReady();
        core.queue += writeSystemRegister( CPU32_SREG_SFC, 5 );  // Core CPU32 configuration
        core.queue += writeSystemRegister( CPU32_SREG_DFC, 5 );
        core.queue += writeMemory( 0xFFFA21, 0x0000, sizeByte ); // SYPCR - Set watchdog enable to 0
        core.queue += writeMemory( 0xFFFA04, 0x7f00, sizeWord ); // SYNCR - Set clock bits
        if ( core.queue.send() == false ) return false;

        ////////////////////////////////////////////////////////////////////
        // Configure chip select and base of all memory mapped, external, devices.

        core.queue  = writeMemory( 0xfffa44, 0x3FFB, sizeWord ); // CSPAR0  - ~
        core.queue += writeMemory( 0xfffa46, 0x0003, sizeWord ); // CSPAR1  - ~
        core.queue += writeMemory( 0xfffa48, 0x0007, sizeWord ); // CSBARBT - Flash. Base 00_0000, size 1M
        core.queue += writeMemory( 0xfffa4a, 0x69f0, sizeWord ); // CSORBT  - ~
        core.queue += writeMemory( 0xfffa4c, 0x2007, sizeWord ); // CSBAR0  - SRAM.  Base 20_0000, size 1M
        core.queue += writeMemory( 0xFFFA4E, 0x7870, sizeWord ); // CSOR0   - ~
        core.queue += writeMemory( 0xfffa50, 0x0007, sizeWord ); // CSBAR1  - Flash. Base 00_0000, size 1M
        core.queue += writeMemory( 0xfffa52, 0x31F0, sizeWord ); // CSOR1   - ~
        core.queue += writeMemory( 0xFFFA54, 0x0007, sizeWord ); // CSBAR2  - Flash. Base 00_0000, size 1M
        core.queue += writeMemory( 0xFFFA56, 0x51F0, sizeWord ); // CSOR2   - ~
        if ( core.queue.send() == false ) return false;

        core.queue  = writeMemory( 0xFFFA58, 0xF000, sizeWord ); // CSBAR3  - ~
        core.queue += writeMemory( 0xFFFA5A, 0x77F0, sizeWord ); // CSOR3   - ~
        core.queue += writeMemory( 0xFFFA5C, 0xF008, sizeWord ); // CSBAR4  - ~
        core.queue += writeMemory( 0xFFFA5E, 0x6BF0, sizeWord ); // CSOR4   - ~
        core.queue += writeMemory( 0xFFFA60, 0xF008, sizeWord ); // CSBAR5  - ~
        core.queue += writeMemory( 0xFFFA62, 0x73F0, sizeWord ); // CSOR5   - ~
        core.queue += writeMemory( 0xFFFA64, 0xF001, sizeWord ); // CSBAR6  - ~
        core.queue += writeMemory( 0xFFFA66, 0x7A30, sizeWord ); // CSOR6   - ~

        core.queue += writeMemory( 0xFFFA68, 0xFFF8, sizeWord ); // CSBAR7  - ~
        core.queue += writeMemory( 0xFFFA6A, 0xABC7, sizeWord ); // CSOR7   - ~
        if ( core.queue.send() == false ) return false;

        core.queue  = writeMemory( 0xFFFB04, 0x1000, sizeWord ); // TPURAM at 10_0000
        core.queue += writeMemory( 0xFFFA1F, 0x0008, sizeByte ); // PortE
        core.queue += writeMemory( 0xFFFA1B, 0x00f8, sizeByte );
        core.queue += writeMemory( 0xFFFA1D, 0x00f5, sizeByte );
        if ( core.queue.send() == false ) return false;

        core.queue  = readMemory( 0x000000, sizeDword ); // Flash
        core.queue += readMemory( 0x100000, sizeWord  ); // TPURAM
        core.queue += readMemory( 0x200000, sizeWord  ); // SRAM
        if ( core.queue.send() == false ) {
            core.castMessage("Error: trionic_5::init - Unable to perform read check on memory regions");
            return false;
        }

        if ( core.getData( &buf[0], TAP_DO_READMEMORY, sizeDword, 0 ) &&
             core.getData( &buf[2], TAP_DO_READMEMORY, sizeWord , 1 ) &&
             core.getData( &buf[3], TAP_DO_READMEMORY, sizeWord , 2 ) ) {
            for ( uint32_t i = 0; i < 4; i++ )
                core.castMessage("Info: [ %u ] - %04X", i, buf[i] );
        } else {
            core.castMessage("Error: trionic_5::init - Unable to retrieve read checks from memory regions");
            return false;
        }

        config.Frequency = 6000000;
        return core.queue.send( setInterface( config ) );
    }
};


class trionic_7
    : public iTrionic {
public:
    trionic_7(bdmstuff &p)
        : requests(p), CPU32_requests(p), iTrionic(p) { }
    ~trionic_7() { }

    bool write( const target_t *target , const memory_t *region ) {
        if ( region->type == opFlash )
            return writeFlash_mk2( txTrionic7, target, region );
        else if ( region->type == opSRAM )
            return writeSRAM( txTrionic7, target, region );
        return false;
    }

    bool init(const target_t *, const memory_t *) {
        TAP_Config_host_t config;
        uint16_t buf[ 4 ];

        core.castMessage("Info: trionic_7::init");

        config.Type = TAP_IO_BDMOLD;
        config.Frequency = 4000000;
        config.cfgmask.Endian = TAP_BIGENDIAN;

        // Interface config
        core.queue  = setInterface( config );
        core.queue += targetReset();
        core.queue += targetReady();
        core.queue += writeSystemRegister( CPU32_SREG_SFC, 5 );  // Core CPU32 configuration
        core.queue += writeSystemRegister( CPU32_SREG_DFC, 5 );
        core.queue += writeMemory( 0xFFFA21, 0x0000, sizeByte ); // SYPCR - Set watchdog enable to 0
        core.queue += writeMemory( 0xFFFA04, 0x7f00, sizeWord ); // SYNCR - Set clock bits
        if ( core.queue.send() == false ) return false;

        ////////////////////////////////////////////////////////////////////
        // Configure chip select and base of all memory mapped, external, devices.

        core.queue  = writeMemory( 0xfffa44, 0x2fff, sizeWord ); // CSPAR0  - cs5 as 8-bit chip select, cs4,3,2,1,0,boot as 16-bit chip select
        core.queue += writeMemory( 0xfffa46, 0x0001, sizeWord ); // CSPAR1  - cs6 as addr[19], The rest as discrete or alt. 339 has other hardcoded straps than 332 so hard to tell
        core.queue += writeMemory( 0xfffa48, 0x0007, sizeWord ); // CSBARBT - Flash. Base 00_0000, size 1M
        core.queue += writeMemory( 0xfffa4a, 0x6bb0, sizeWord ); // CSORBT  - Flash. MODE: Async, BYTE: Both, RW: Read , STRB: AS, DSACK: Fast, SPACE: SU/U, IPL: All, AVEC: Off

        core.queue += writeMemory( 0xfffa4c, 0x2007, sizeWord ); // CSBAR0  - SRAM.  Base 20_0000, size 1M
        core.queue += writeMemory( 0xfffa4e, 0x6830, sizeWord ); // CSOR0   - SRAM.  MODE: Async, BYTE: Both, RW: Read , STRB: AS, DSACK: 0 wt, SPACE: SU/U, IPL: All, AVEC: Off
        core.queue += writeMemory( 0xfffa50, 0x0007, sizeWord ); // CSBAR1  - Flash. Base 00_0000, size 1M
        core.queue += writeMemory( 0xfffa52, 0x7030, sizeWord ); // CSOR1   - Flash. MODE: Async, BYTE: Both, RW: Write, STRB: AS, DSACK: 0 wt, SPACE: SU/U, IPL: All, AVEC: Off
        if ( core.queue.send() == false ) return false;

        // CSBAR2 / CSOR2 are unused (Connects to BGACK)
        // CSBAR6 / CSOR6 -> 8, 10 are unused. (addr[9], io signals etc)
        core.queue  = writeMemory( 0xfffa58, 0x2007, sizeWord ); // CSBAR3  - SRAM.  Base 20_0000, size 1M
        core.queue += writeMemory( 0xfffa5a, 0x5030, sizeWord ); // CSOR3   - SRAM.  MODE: Async, BYTE: High, RW: Write, STRB: AS, DSACK: 0 wt, SPACE: SU/U, IPL: All, AVEC: Off
        core.queue += writeMemory( 0xfffa5c, 0x2007, sizeWord ); // CSBAR4  - SRAM.  Base 20_0000, size 1M
        core.queue += writeMemory( 0xfffa5e, 0x3030, sizeWord ); // CSOR4   - SRAM.  MODE: Async, BYTE: Low , RW: Write, STRB: AS, DSACK: 0 wt, SPACE: SU/U, IPL: All, AVEC: Off

        core.queue += writeMemory( 0xfffa60, 0xff00, sizeWord ); // CSBAR5  - CAN.   Base FF_0000, size 2k    ( CAN controller is a weird one... )
        core.queue += writeMemory( 0xfffa62, 0x7bf0, sizeWord ); // CSOR5   - CAN.   MODE: Async, BYTE: Both, RW: Both , STRB: AS, DSACK: Ext , SPACE: SU/U, IPL: All, AVEC: Off
        core.queue += writeMemory( 0xfffa70, 0xfff8, sizeWord ); // CSBAR9  - acRLY. Base FF_F800, size 2k    ( Suspect this is debug code they forgot to remove? )
        core.queue += writeMemory( 0xfffa72, 0x2bc7, sizeWord ); // CSOR9   - acRLY. MODE: Async, BYTE: Low , RW: Read , STRB: AS, DSACK: Ext , SPACE: CPU , IPL: 3up, AVEC: On
        if ( core.queue.send() == false ) return false;

        core.queue  = writeMemory( 0xFFFB04, 0x1000, sizeWord ); // TPURAM at 10_0000
        core.queue += writeMemory( 0xfff706, 0x1000, sizeWord ); // Latch up external power
        core.queue += writeMemory( 0xfffa41, 0x001f, sizeByte ); // PC0,1. CS3,4 SRAM W/E   // Configure SRAM and junk
        if ( core.queue.send() == false ) return false;

        core.queue  = readMemory( 0x000000, sizeDword ); // Flash
        core.queue += readMemory( 0x100000, sizeWord  ); // TPURAM
        core.queue += readMemory( 0x200000, sizeWord  ); // SRAM
        if ( core.queue.send() == false ) {
            core.castMessage("Error: trionic_7::init - Unable to perform read check on memory regions");
            return false;
        }

        if ( core.getData( &buf[0], TAP_DO_READMEMORY, sizeDword, 0 ) &&
             core.getData( &buf[2], TAP_DO_READMEMORY, sizeWord , 1 ) &&
             core.getData( &buf[3], TAP_DO_READMEMORY, sizeWord , 2 ) ) {
            for ( uint32_t i = 0; i < 4; i++ )
                core.castMessage("Info: [ %u ] - %04X", i, buf[i] );
        } else {
            core.castMessage("Error: trionic_7::init - Unable to retrieve read checks from memory regions");
            return false;
        }

        config.Frequency = 6000000;
        return core.queue.send( setInterface( config ) );
    }
};


class trionic_8
    : public iTrionic {
public:
    trionic_8(bdmstuff &p)
        : requests(p), CPU32_requests(p), iTrionic(p) { }
    ~trionic_8() { }

    bool write( const target_t *target , const memory_t *region ) {
        if ( region->type == opFlash )
            return writeFlash( enWidth16, 1, target, region );
        else if ( region->type == opSRAM )
            return writeSRAM( txTrionic8, target, region );
        return false;
    }

    bool init(const target_t *, const memory_t *) {

        TAP_Config_host_t config;

        core.castMessage("Info: trionic_8::init");

        config.Type = TAP_IO_BDMOLD;
        config.Frequency = 1500000;
        config.cfgmask.Endian = TAP_BIGENDIAN;

        // Interface config
        core.queue  = setInterface( config );
        core.queue += targetReset();
        core.queue += targetReady();
        core.queue += writeSystemRegister( CPU32_SREG_SFC, 5 );  // Core CPU32 configuration
        core.queue += writeSystemRegister( CPU32_SREG_DFC, 5 );
        core.queue += writeMemory( 0xFFFA00, 0xF14F, sizeWord ); // Module Config register

        // This is actually PortE...
        // core.queue += writeMemory( 0xFFFA27, 0x0055, sizeByte );
        core.queue += writeMemory( 0xFFFA27, 0x00AA, sizeByte );
        
        // core.queue += writeMemory( 0xFFFA55, 0x0055, sizeByte ); // Trigger watchdog once before disabling it
        // core.queue += writeMemory( 0xFFFA55, 0x00AA, sizeByte ); 
        // core.queue += writeMemory( 0xFFFA55, 0x0055, sizeByte ); // Trigger watchdog once before disabling it
        // core.queue += writeMemory( 0xFFFA55, 0x00AA, sizeByte ); 
        core.queue += writeMemory( 0xFFFA58, 0x0000, sizeWord ); // SWI - Disable that bastard
        core.queue += writeMemory( 0xFFFA08, 0x0008, sizeWord ); // SYNCR - Set clock bits
        if ( core.queue.send() == false ) return false;

        sleep_ms( 50 );

        if ( !core.queue.send( writeMemory( 0xFFFA08, 0x6008, sizeWord ) ) )
            return false;

        sleep_ms( 50 );


        core.queue  = writeMemory( 0xFFFA7C, 0x0004, sizeWord ); // Flash config
        core.queue += writeMemory( 0xFFFA4C, 0x0C00, sizeWord ); // Flash config
        core.queue += writeMemory( 0xFFFA7E, 0xF332, sizeWord ); // Flash config



        core.queue  = writeMemory( 0xFFF6C0, 0x8400, sizeWord );
        core.queue += writeMemory( 0xFFFA1F, 0x00ff, sizeByte );


        core.queue += writeMemory( 0xFFFA36, 0x5577, sizeWord );
        core.queue += writeMemory( 0xFFFA35, 0x0030, sizeByte );
        core.queue += writeMemory( 0xFFFA3B, 0x0000, sizeByte );
        core.queue += writeMemory( 0xFFFA3C, 0x0000, sizeByte );
        core.queue += writeMemory( 0xFFFA3A, 0x0004, sizeByte );


        // Should be this according to boot but it's crashing..

        // Configure PEPAR at fffa27
        // 7:6 - 10 = PE[7:5] as FC[2:0]
        //   5 -  0 ( Reserved )
        //   4 -  1 = PE4 as clock out
        //   3 -  1 = PE3 as SIZE/BLOCK
        //   2 -  1 = PE2 as _AS_  ( Address Strobe )
        //   1 -  1 = PE1 as _DS_  ( Data Strobe )
        //   0 -  1 = PE0 as R/_W_ ( Read / _Write_ )
        // core.queue += writeMemory( 0xFFFA27, 0x009f, sizeByte );

        if ( core.queue.send() == false ) return false;

        core.queue  = writeMemory( 0xFFF680, 0x8000, sizeWord );
        core.queue += writeMemory( 0xFFF680, 0x0000, sizeWord );
        core.queue += writeMemory( 0xFFF684, 0x1000, sizeWord );

        if ( core.queue.send() == false ) return false;

        config.Frequency = 6000000;
        return core.queue.send( setInterface( config ) );
    }
};


class trionic_8mcp
    : public iTrionic {

    bool dumpMCP( const memory_t *region ) {

        memory_t memSpec = { opFlash };

        if ( region == nullptr ) {
            core.castMessage("Error: This routine needs to know base address");
            return false;
        }

        if ( region->type != opFlash ) {
            core.castMessage("Error: This routine only knows how to deal with CMFI");
            return false;
        }

        if ( !cmfi.setShadow( false ) )
            return false;

        memSpec.address = region->address;
        memSpec.size = 0x40000;
        core.setRange( &memSpec );

        // Dump main portion
        if ( core.queue.send( assistDump( 0, 0x40000 ) ) == false ) {
             core.castMessage("Error: Unable to dump main flash");
            return false;
        }

        if ( !cmfi.setShadow( true ) )
            return false;

        memSpec.size = 256;
        core.setRange( &memSpec );

        if ( core.queue.send( assistDump( 0, 256 ) ) == false ) {
            core.castMessage("Error: Unable to dump shadow");
            return false;
        }

        return core.swapDump( 4 );
    }

public:
    trionic_8mcp(bdmstuff &p)
        : requests(p), CPU32_requests(p), iTrionic(p) { }
    ~trionic_8mcp() { }

    // MC68F375 is complex with shadow regions and other annoying stuff so the generic stuff won't do
    bool read(const target_t *, const memory_t *mem) {

        if ( mem->type == opFlash )
            return dumpMCP( mem );

        // Treat everything else as regular reads
        if ( core.queue.send( assistDump( mem->address, mem->size ) ) == false ) {
            core.castMessage("Info: trionic_8mcp::read - Unable to start dump");
            return false;
        }

        return core.swapDump( 4 );
    }

    bool write( const target_t *target , const memory_t *region ) {

        if ( region->type == opFlash )
            return writeCMFI( region, enCPU32_CMFI_V51 );

        else if ( region->type == opSRAM )
            return writeSRAM( txTrionic8mcp, target, region );

        return false;
    }

    bool init(const target_t *, const memory_t *) {

        TAP_Config_host_t config;
        uint16_t buf[ 4 ];

        core.castMessage("Info: trionic_8mcp::init()");

        config.Type = TAP_IO_BDMOLD;
        config.Frequency = 1000000;
        config.cfgmask.Endian = TAP_BIGENDIAN;

        // Interface config
        core.queue  = setInterface( config );
        core.queue += targetReset();
        core.queue += targetReady();
        core.queue += writeSystemRegister( CPU32_SREG_SFC, 5 );  // Core CPU32 configuration
        core.queue += writeSystemRegister( CPU32_SREG_DFC, 5 );
        core.queue += writeMemory( 0xFFFA21, 0x0070, sizeByte ); // SYPCR - Set watchdog enable to 0, SWP to 1 and SWT to 3
        // 24 MHz                                                // Ie crank the divider to 16777216 since it's not listening to the disable signal...
        //
        // X: 1
        // Y: 0
        // W: 5
        core.queue += writeMemory( 0xFFFA04, 0xD080, sizeWord ); // SYNCR - Set clock bits  (Used to be 0xD084)
        if ( core.queue.send() == false ) return false;

        sleep_ms( 5 );

        config.Frequency = 3000000;

        core.queue  = setInterface( config );
        core.queue += writeMemory( 0xFFF800, 0xC800, sizeWord ); // CMFIMCR - Set STOP and PROTECT bit. LOCK to false
        core.queue += writeMemory( 0xFFF808, 0x0000, sizeWord ); // CMFIBAH - Base flash at 00_0000
        core.queue += writeMemory( 0xFFF80A, 0x0000, sizeWord ); // CMFIBAL -/
        core.queue += writeMemory( 0xFFF800, 0x4800, sizeWord ); // CMFIMCR - Enable flash

        core.queue += writeMemory( 0xFFF884, 0x1000, sizeWord ); // DPTBAR  - Base DPTRAM at 10_0000
        core.queue += writeMemory( 0xFFF880, 0x0000, sizeWord ); // DPTMCR  - Enable DPTRAM

        core.queue += writeMemory( 0xFFFB04, 0x0020, sizeWord ); // RAMBAH  - Base SRAM at 20_0000
        core.queue += writeMemory( 0xFFFB06, 0x0000, sizeWord ); // RAMBAL  -/
        core.queue += writeMemory( 0xFFFB00, 0x0800, sizeWord ); // RAMMCR  - Enable SRAM
        if ( core.queue.send() == false ) return false;

        core.queue  = readMemory( 0x000000, sizeDword ); // Flash
        core.queue += readMemory( 0x100000, sizeWord  ); // DPTRAM
        core.queue += readMemory( 0x200000, sizeWord  ); // SRAM
        if ( core.queue.send() == false ) {
            core.castMessage("Error: trionic_8mcp::init - Unable to perform read check on memory regions");
            return false;
        }

        if ( core.getData( &buf[0], TAP_DO_READMEMORY, sizeDword, 0 ) &&
             core.getData( &buf[2], TAP_DO_READMEMORY, sizeWord , 1 ) &&
             core.getData( &buf[3], TAP_DO_READMEMORY, sizeWord , 2 ) ) {
            for ( uint32_t i = 0; i < 4; i++ )
                core.castMessage("Info: [ %u ] - %04X", i, buf[i] );
        } else {
            core.castMessage("Error: trionic_8mcp::init - Unable to retrieve read checks from memory regions");
            return false;
        }

        return true;
    }
};


iTarget *instTechII(bdmstuff &core) {
    return new gm_techII( core );
}

iTarget *instTrionic5(bdmstuff &core) {
    return new trionic_5( core );
}

iTarget *instTrionic7(bdmstuff &core) {
    return new trionic_7( core );
}

iTarget *instTrionic8(bdmstuff &core) {
    return new trionic_8( core );
}

iTarget *instTrionic8mcp(bdmstuff &core) {
    return new trionic_8mcp( core );
}

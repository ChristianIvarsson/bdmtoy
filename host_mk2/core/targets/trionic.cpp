//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Trionic related code

#include "../bdmstuff.h"

#include "targets.h"

#include "../requests_cpu32.h"

#include "drivers/CPU32/trionic/txdriver.h"

class iTrionic
    : public requests_cpu32 {

    bool driverDemand( uint32_t what ) {
        bool retval;
        uint16_t status;
        uint32_t driverResponse;

        // Store what to do
        if ( !core.queue.send(writeDataRegister( 0, what )) ) {
            core.castMessage("Error: driverDemand - Could not write d0");
            return false;
        }

        // Set program counter to 10_0400
        if ( !core.queue.send(writeSystemRegister( 0, 0x100400 )) ) {
            core.castMessage("Error: driverDemand - Could not set program counter");
            return false;
        }

        // Run target resident driver
        if ( !core.queue.send(targetStart()) ) {
            core.castMessage("Error: driverDemand - Could not start target");
            return false;
        }

        // Wait for target stop
        do {
            retval = core.getStatus( &status );
            // Let's be nice. No need to absolutely hammer the status request
            sleep_ms( 50 );
        } while ( retval && status == RET_TARGETRUNNING );

        if ( status != RET_TARGETSTOPPED ) {
            core.castMessage("Error: driverDemand - Could not stop target");
            return false;
        }

        if ( !core.queue.send(readDataRegister( 0 )) ||
             !core.getData( (uint16_t*)&driverResponse, TAP_DO_READREGISTER, 4 ) ) {
            core.castMessage("Error: driverDemand - Could not retrieve driver status");
            return false;
        }

        if ( driverResponse != 1 ) {
            core.castMessage("Error: driverDemand - Driver failed with %08x", driverResponse);
            return false;
        }

        return true;
    }

protected:
    bool writeFlash( const target_t *, const memory_t * ) {

        uint16_t idTemp[ 8 ];
        uint32_t flashType, flashSize;
        memory_t memSpec = { opFlash };

        // Upload driver
        core.castMessage("Info: Uploading driver..");

        if ( fillDataBE4(0x100400, txDriver, sizeof(txDriver)) == false ) {
            core.castMessage("Info: iTrionic::write - Unable upload driver");
            return false;
        }

        core.setTimeout( 61 * 1000 );

        if ( driverDemand( 3 ) == false )
            return false;

        core.castMessage("Info: Retrieving flash info..");

        core.queue  = readDataRegister( 7 );    // MID / DID
        core.queue += readDataRegister( 3 );    // EID
        core.queue += readDataRegister( 6 );    // Type
        core.queue += readAddressRegister( 1 ); // Size

        if ( core.queue.send() == false ||
             core.getData( &idTemp[0], TAP_DO_READREGISTER, 4, 0 ) == false ||
             core.getData( &idTemp[2], TAP_DO_READREGISTER, 4, 1 ) == false ||
             core.getData( &idTemp[4], TAP_DO_READREGISTER, 4, 2 ) == false ||
             core.getData( &idTemp[6], TAP_DO_READREGISTER, 4, 3 ) == false ) {
            core.castMessage("Error: trionic_7::write - Unable to retrieve flash information");
            return false;
        }

        core.castMessage("Info: MID      %02X", (idTemp[0]>>8)&0xFF);
        core.castMessage("Info: DID      %02X", (idTemp[0])   &0xFF);
        core.castMessage("Info: EID    %04X"  ,  idTemp[2]);
        // core.castMessage("Info: Type     %02X", *(uint32_t *) &idTemp[4]);
        core.castMessage("Info: size %06X"    , *(uint32_t *) &idTemp[6]); 

        flashType = *(uint32_t *) &idTemp[4];
        flashSize = *(uint32_t *) &idTemp[6];

        switch ( flashType ) {
        case 1:  core.castMessage("Info: Old H/W flash"); break;
        case 2:  core.castMessage("Info: Modern toggle flash"); break;
        case 3:  core.castMessage("Info: Disgusting Atmel flash.."); break;
        default: core.castMessage("Error: Driver does not understand this flash"); return false;
        }

        if ( core.fileSize != flashSize ) {
            core.castMessage("Error: File size does not match expected size");
            return false;
        }

        /*
        if (core.fileSize > flashSize)
        {
            core.castMessage("It looks like your file is too big for the target. Aborting..");
            return false;
        }
        // Only do this on T5 and T7, the ones where it's actually possible to add more flash. Implement checks before adding the other ones!
        else if ( core.fileSize < flashSize ) {
            core.castMessage("Info: File is too small. Checking if it can be mirrored..");
            if ( core.mirrorReadFile( flashSize ) == false )
                return false;
            core.castMessage("Info: It could! Let's go");
        }
        */

        // Notify worker about our intended address range
        memSpec.size = flashSize;
        core.setRange( &memSpec );

        // Big endian target so buffer must be swapped
        core.swapBuffer( 4, core.fileSize );

        core.castMessage("Info: Erasing flash..");
        if ( driverDemand( 2 ) == false )
            return false;

        // Set destination address for data
        if ( !core.queue.send(writeAddressRegister( 0, memSpec.address )) ) {
            core.castMessage("Error: Could not update destination pointer");
            return false;
        }

        core.castMessage("Info: Writing flash");
        if ( !core.queue.send(assistFlash(
                                memSpec.address, memSpec.size,
                                0x100400, 0x100000, 0x400)) ) {
            core.castMessage("Error: Flash failed");
            return false;
        }

        return core.queue.send(targetReset());
    }

public:
    iTrionic(bdmstuff & p)
        : requests_cpu32(p) { }

    virtual bool read(const target_t *, const memory_t *mem) {
        if ( core.queue.send( assistDump( mem->address, mem->size ) ) == false ) {
            core.castMessage("Info: iTrionic::read - Unable to start dump");
            return false;
        }
        return core.swapDump( 4 );
    }

    virtual bool write( const target_t *target , const memory_t *region ) {
        if ( region->type == opFlash )
            return writeFlash( target, region );
        return false;
    }

    ~iTrionic() { }
};

class trionic_5
    : public iTrionic {
public:
    trionic_5( bdmstuff & core )
        : iTrionic( core ) { }
    ~trionic_5() { }
};

class trionic_7
    : public iTrionic {
public:
    trionic_7(bdmstuff &core)
        : iTrionic(core) { }

    bool init(const target_t *, const memory_t *)
    {
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
        core.queue += writeSystemRegister( CPU32_SREG_SFC, 5 ); // Core CPU32 configuration
        core.queue += writeSystemRegister( CPU32_SREG_DFC, 5 );
        core.queue += writeMemory( 0xFFFA04, 0x7f00, sizeWord ); // SYNCR - Set clock bits
        core.queue += writeMemory( 0xFFFA21, 0x0000, sizeByte ); // SYPCR - Set watchdog enable to 0
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
            core.castMessage("Info: trionic_7::init - Unable to perform read check on memory regions");
            return false;
        }

        if ( core.getData( &buf[0], TAP_DO_READMEMORY, 4, 0 ) &&
             core.getData( &buf[2], TAP_DO_READMEMORY, 2, 1 ) &&
             core.getData( &buf[3], TAP_DO_READMEMORY, 2, 2 ) ) {
            for ( uint32_t i = 0; i < 4; i++ )
                core.castMessage("Info: [ %u ] - %04X", i, buf[i] );
        } else {
            core.castMessage("Info: trionic_7::init - Unable to retrieve read checks from memory regions");
            return false;
        }

        config.Frequency = 6000000;
        return core.queue.send( setInterface( config ) );
    }

    ~trionic_7() { }
};

iTarget *instTrionic5(bdmstuff &core) {
    return new trionic_5( core );
}

iTarget *instTrionic7(bdmstuff &core) {
    return new trionic_7(core);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Trionic related code

#include "../bdmstuff.h"

#include "targets.h"
#include "generic.h"

#include "../requests_cpu32.h"

class iTrionic
    : public iTarget,
      public requests_cpu32
{
protected:
    bool writeFlash()
    {
        return true;
    }

public:
    iTrionic(bdmstuff &core)
        : iTarget(core)
    {}

    bool read(const target_t *trg, const memory_t *mem) {
        core.setRange( mem );
        if ( core.queue.send( assistDump( mem->address, mem->size ) ) == false ) {
            core.castMessage("Info: iTrionic::read - Unable to start dump");
            return false;
        }
        return core.swapBuffer( 4 );
    }

    ~iTrionic()
    {
        // printf("Info: ~iTrionic()\n");
    }
};

class trionic_5
    : public iTrionic {
public:
    trionic_5( bdmstuff & core )
        : iTrionic( core ) {
    }
    ~trionic_5() {
        printf("Info: ~trionic_5()\n");
    }
};

class trionic_7
    : public iTrionic
{
public:
    trionic_7(bdmstuff &core)
        : iTrionic(core)
    { }

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

    ~trionic_7() {
        // printf("Info: ~trionic_7()\n");
    }
};

iTarget *instTrionic5(bdmstuff &core)
{
    return new trionic_5( core );
}

iTarget *instTrionic7(bdmstuff &core)
{
    return new trionic_7(core);
}

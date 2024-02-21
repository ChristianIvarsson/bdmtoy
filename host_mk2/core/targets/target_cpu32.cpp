//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Random CPU32 targets

#include <cstring>
#include "../bdmstuff.h"
#include "targets.h"
#include "../requests_cpu32.h"
#include "utils/cpu32/utils_cpu32.h"

class iCPU32
    : public cpu32_utils, public virtual requests_cpu32, public iTarget {

protected:
    bool writeSRAM( const target_t *, const memory_t *region ) {
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
    iCPU32(bdmstuff & p)
        : requests(p), requests_cpu32(p), cpu32_utils(p) { }
    ~iCPU32() { }

    virtual bool read(const target_t *, const memory_t *mem) {
        core.castMessage("iCPU32::read()");
        if ( core.queue.send( assistDump( mem->address, mem->size ) ) == false ) {
            core.castMessage("Info: iCPU32::read - Unable to start dump");
            return false;
        }
        return core.swapDump( 4 );
    }
};

// MC68376
class VolvoS60ACC
    : public iCPU32 {
public:
    VolvoS60ACC(bdmstuff &p)
        : requests(p), requests_cpu32(p), iCPU32(p) { }
    ~VolvoS60ACC() { }

    bool write( const target_t *target , const memory_t *region ) {
        if ( region->type == opFlash )
            return genericFlash( enWidth16, 1, target, region );
        else if ( region->type == opSRAM )
            return writeSRAM( target, region );
        return false;
    }

    bool init(const target_t *, const memory_t *)
    {
        TAP_Config_host_t config;

        core.castMessage("Info: VolvoS60ACC::init");

        config.Type = TAP_IO_BDMOLD;
        config.Frequency = 4000000;
        config.cfgmask.Endian = TAP_BIGENDIAN;

        // Interface config
        core.queue  = setInterface( config );
        core.queue += targetReset();
        core.queue += targetReady();
        core.queue += writeSystemRegister( CPU32_SREG_SFC, 5 );  // Core CPU32 configuration
        core.queue += writeSystemRegister( CPU32_SREG_DFC, 5 );
        core.queue += writeMemory( 0xFFFA04, 0x7f00, sizeWord ); // SYNCR - Set clock bits
        core.queue += writeMemory( 0xFFFA21, 0x0000, sizeByte ); // SYPCR - Set watchdog enable to 0
        if ( core.queue.send() == false ) return false;

        ////////////////////////////////////////////////////////////////////
        // Most stuff has been taken straight from the target with the exception of memory size and its base address.
        // Doing some sort of standard where flash is at 0, sram at 20_0000 and tpuram at 10_0000

        core.queue  = writeMemory( 0xfffa44, 0x00AF, sizeWord ); // CSPAR0  -
        // - no CSPAR1 -
        core.queue += writeMemory( 0xfffa48, 0x0007, sizeWord ); // CSBARBT - Flash. Base 00_0000, size 1M
        core.queue += writeMemory( 0xfffa4a, 0x6830, sizeWord ); // CSORBT  -
        core.queue += writeMemory( 0xfffa4c, 0x0007, sizeWord ); // CSBAR0  - Flash. Base 00_0000, size 1M
        core.queue += writeMemory( 0xfffa4e, 0x7430, sizeWord ); // CSOR0   -

        core.queue += writeMemory( 0xfffa50, 0x2007, sizeWord ); // CSBAR1  - SRAM.  Base 20_0000, size 1M
        core.queue += writeMemory( 0xfffa52, 0x5030, sizeWord ); // CSOR1   -
        core.queue += writeMemory( 0xFFFA54, 0x2007, sizeWord ); // CSBAR2  - SRAM.  Base 20_0000, size 1M
        core.queue += writeMemory( 0xFFFA56, 0x4830, sizeWord ); // CSOR2   -
        if ( core.queue.send() == false ) return false;

        core.queue  = writeMemory( 0xFFFB04, 0x1000, sizeWord ); // TPURAM at 10_0000
        if ( core.queue.send() == false ) return false;

        config.Frequency = 6000000;
        return core.queue.send( setInterface( config ) );
    }
};

iTarget *instVolvoS60ACC(bdmstuff &core) {
    return new VolvoS60ACC( core );
}
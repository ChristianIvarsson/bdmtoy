#ifndef __UTILS_CPU32_H__
#define __UTILS_CPU32_H__

// Target-specific driver.. stuff
#include "../../../bdmstuff.h"
#include "../../../requests_cpu32.h"
#include "../../../utils/crypto.h"

#include "../utils_shared.h"
#include "../../partition_helper.h"


#include "priv/cpu32_cmfi.h"
#include "priv/cpu32_md5.h"
#include "priv/cpu32_flash.h"

class cpu32_utils
    : private CPU32_genmd5, private CPU32_genflash, private CPU32_gencmfi {

protected:
    CPU32_genflash &flash;
    CPU32_genmd5 &md5;
    CPU32_gencmfi &cmfi;

public:
    explicit cpu32_utils(bdmstuff &p)
        : requests(p), CPU32_requests(p),
            CPU32_genmd5(p), CPU32_genflash(p), CPU32_gencmfi(p), flash(*this), md5(*this), cmfi(*this)
    {
        printf("cpu32_utils()\n");
    }

    bool writeFlash(
                    eFlashWidth width,
                    uint32_t nChips,
                    const target_t *,
                    const memory_t *region,
                    uint32_t driverAddress = 0x100000,
                    uint32_t bufferAddress = 0x100400 ) {

        md5k_t remoteKeys, localKeys;
        flashid_t fID = { 0 };
        crypto::md5 local_md5;

        if ( region == nullptr ) {
            core.castMessage("Error: This routine needs to know base address");
            return false;
        }

        if ( region->type != opFlash ) {
            core.castMessage("Error: This routine only knows how to deal with flash");
            return false;
        }

        // Detect flash type
        uint32_t flashBase = region->address;

        if ( !flash.detect_mk2( fID, flashBase, width, nChips ) )
            return false;

        // Get address map of this flash
        const flashpart_t *part = parthelper::getMap( fID.MID, fID.DID, width);

        if ( part == nullptr ) {
            core.castMessage("Error: Host does not understand this flash");
            return false;
        }

        if ( part->count > 32 ) {
            core.castMessage("Error: Too many partitions");
            return false;
        }

        core.setTimeout( GLOBALTIMEOUT * 1000 );

        core.castMessage("Info: Comparing md5..");

        uint32_t mask = 0;
        if ( !md5.upload( driverAddress, true ) )
            return false;

        core.setTimeout( 61 * 1000 );

        for ( size_t i = 0; i < part->count; i++ ) {

            uint32_t start  = (i == 0) ? 0 : (part->partitions[ i - 1 ] * nChips);
            uint32_t length = (part->partitions[ i ] * nChips) - start;

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

        if ( !flash.upload(part, driverAddress, bufferAddress, nChips) )
            return false;

        if ( !flash.erase(mask) )
            return false;

        return ( flash.write(mask) && core.queue.send( targetReset() ) );
    }

    bool writeCMFI(
                    const memory_t *region,
                    const cpu32_cmfi_ver version,
                    uint32_t driverAddress = 0x100000,
                    uint32_t bufferAddress = 0x100800 ) {
        crypto::md5 local_md5;
        uint32_t flashBase;
        md5k_t remoteKeys;
        md5k_t localKeys;
        uint32_t mask = 0;

        if ( region == nullptr ) {
            core.castMessage("Error: This routine needs to know base address");
            return false;
        }

        if ( region->type != opFlash ) {
            core.castMessage("Error: This routine only knows how to deal with CMFI");
            return false;
        }

        if ( core.fileSize != ((256 * 1024) + 256) ) {
            core.castMessage("Error: Incorrect file size");
            return false;
        }

        // - Implement detection and generation of shadow data?
        // - Should be fairly easy to detect flash base by loking for movec to vbr
        // - How to deal with the rest, however

        core.setTimeout( 61 * 1000 );

        flashBase = region->address;

        md5.upload( CPU32_f375_md5, sizeof(CPU32_f375_md5), driverAddress);

        if ( !cmfi.setShadow( false ) )
            return false;

        for ( size_t i = 0; i < 9; i++ ) {

            uint32_t start  = (i << 15);
            uint32_t length = (i == 8) ? 256 : 32768;

            local_md5.hash( &localKeys, &core.buffer[ start ], length );

            // Flash base does not necessarily sit at 0 so this has to be appended after the local file has been hashed
            start &= 0x3ffff;
            start += flashBase;

            if ( i == 8 && !setShadow( true ) )
                    return false;

            if ( !md5.hash( &remoteKeys, start, length, true ) )
                return false;

            if ( memcmp( &remoteKeys, &localKeys, sizeof(md5k_t) ) != 0 ) {
                mask |= ( 1 << i );
                core.castMessage("Info: Sector %u (%06x - %06x) - different", i, start, start + length - 1);
            } else {
                core.castMessage("Info: Sector %u (%06x - %06x) - identical", i, start, start + length - 1);
            }
        }

        if ( !cmfi.setShadow( false ) )
            return false;

        // Bit 8 is actually part of partition 0 so the mask must be modified accordingly
        if ( (mask & 0x101) )
            mask |= 0x101;
/*
        if ( mask == 0 ) {
            core.castMessage("Info: < Forcing mask to all partitions >");
            mask = 0x1ff;
        }
*/
        core.castMessage("Info: Mask is %03x", mask);

        if ( mask == 0 ) {
            core.castMessage("Info: Everything is identical");
            return true;
        }

        if ( !core.swapBuffer( 4, core.fileSize ) )
            return false;

        // Upload and init driver
        if ( !cmfi.upload(region,
                          driverAddress,
                          bufferAddress,
                          version) )
            return false;
        
        if ( !cmfi.sectorErase(mask & 0xff) )
            return false;

        if ( !cmfi.write( mask ) )
        {
            helper_CPU32 hlp( core );
            hlp.printRegisters();
            return false;
        }

        return true;
    }
};

#endif

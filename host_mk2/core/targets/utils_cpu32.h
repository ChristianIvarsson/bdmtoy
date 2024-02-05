#ifndef __UTILS_CPU32_H__
#define __UTILS_CPU32_H__

// Target-specific driver.. stuff
#include "../bdmstuff.h"
#include "../requests_cpu32.h"
#include "../utils/crypto.h"

#include "utils_shared.h"
#include "partition_helper.h"

// Target drivers
#include "drivers/CPU32/generic/flash/genflash.h"
#include "drivers/CPU32/generic/md5/md5.h"

class CPU32_genmd5 : public virtual requests_cpu32 {
    bdmstuff & mdCore;
    uint32_t driverBase;
public:
    explicit CPU32_genmd5(bdmstuff &p)
        : requests(p), requests_cpu32(p), mdCore(p) {
        driverBase = 0;
    }

    bool upload(uint32_t address, bool silent = false) {
        driverBase = address;

        // Upload driver
        if ( !silent )
            mdCore.castMessage("Info: Uploading md5 hash driver..");

        if ( fillDataBE4(address, CPU32_md5, sizeof(CPU32_md5)) == false ) {
            mdCore.castMessage("Error: Unable to upload driver");
            return false;
        }

        return true;
    }

    bool hash(md5k_t *keys, uint32_t start, uint32_t length, bool silent = false) {
        uint16_t status;
        bool retVal;

        if ( keys == nullptr ) {
            mdCore.castMessage("Error: md5 - Need a key buffer");
            return false;
        }

        mdCore.queue  = writeAddressRegister(0, start);
        mdCore.queue += writeDataRegister(0, length);
        mdCore.queue += writeSystemRegister(0, driverBase);
        mdCore.queue += targetStart();

        if ( !mdCore.queue.send() ) {
            mdCore.castMessage("Error: md5 - Unable to configure and/or start driver");
            return false;
        }

        if ( !silent )
            mdCore.castMessage("Info: Hashing..");

        do {
            retVal = mdCore.getStatus(&status);
            sleep_ms(25);
        } while ( retVal && status == RET_TARGETRUNNING );

        if ( status != RET_TARGETSTOPPED ) {
            mdCore.castMessage("Error: md5 - Could not stop target");
            return false;
        }

        mdCore.queue  = readAddressRegister(2);
        mdCore.queue += readAddressRegister(3);
        mdCore.queue += readAddressRegister(4);
        mdCore.queue += readAddressRegister(5);

        if ( mdCore.queue.send() == false ||
             mdCore.getData(&((uint16_t *)keys)[0], TAP_DO_READREGISTER, 4, 0) == false ||
             mdCore.getData(&((uint16_t *)keys)[2], TAP_DO_READREGISTER, 4, 1) == false ||
             mdCore.getData(&((uint16_t *)keys)[4], TAP_DO_READREGISTER, 4, 2) == false ||
             mdCore.getData(&((uint16_t *)keys)[6], TAP_DO_READREGISTER, 4, 3) == false)
        {
            mdCore.castMessage("Error: Unable to retrieve flash information");
            return false;
        }

        // This needs to die
        keys->A = bs32(keys->A);
        keys->B = bs32(keys->B);
        keys->C = bs32(keys->C);
        keys->D = bs32(keys->D);

        return true;
    }
};

class CPU32_genflash : public virtual requests_cpu32 {
        bdmstuff &fCore;
        uint32_t  driverBase;
        uint32_t  flashBase;
        uint32_t  bufferBase;
        uint32_t  chipCount;
        const flashpart_t *flashPart;
        bool flashKnown;
        bool driverInited;

    void clearFlashParams() {
        flashKnown = false;
        flashPart = nullptr;
        chipCount = 1;
    }

    bool doBulkErase(uint32_t start, uint32_t end) {
        uint16_t status;
        bool retVal;

        fCore.castMessage("Info: Erasing %06x - %06x..", start, end - 1);
        // fCore.castMessage("Info: Driver base %08x", driverBase);

        fCore.queue  = writeDataRegister(0, 3);
        fCore.queue += writeAddressRegister(0, start);
        fCore.queue += writeAddressRegister(1, end);
        fCore.queue += writeSystemRegister(0, driverBase);
        fCore.queue += targetStart();

        if ( !fCore.queue.send() ) {
            fCore.castMessage("Error: Unable to configure and/or start erase");
            return false;
        }

        do {
            retVal = fCore.getStatus(&status);
            sleep_ms(25);
        } while ( retVal && status == RET_TARGETRUNNING );

        if ( status != RET_TARGETSTOPPED ) {
            fCore.castMessage("Error: Could not stop target");
            return false;
        }

        return getStatus("erase");
    }

    bool doSectorErase(uint32_t start, uint32_t end) {
        uint16_t status;
        bool retVal;

        fCore.castMessage("Info: Erasing %06x - %06x..", start, end - 1);
        // fCore.castMessage("Info: Driver base %08x", driverBase);

        fCore.queue  = writeDataRegister(0, 2);
        fCore.queue += writeAddressRegister(0, start);
        fCore.queue += writeAddressRegister(1, end);
        fCore.queue += writeSystemRegister(0, driverBase);
        fCore.queue += targetStart();

        if ( !fCore.queue.send() ) {
            fCore.castMessage("Error: Unable to configure and/or start erase");
            return false;
        }

        do {
            retVal = fCore.getStatus(&status);
            sleep_ms(25);
        } while ( retVal && status == RET_TARGETRUNNING );

        if ( status != RET_TARGETSTOPPED ) {
            fCore.castMessage("Error: Could not stop target");
            return false;
        }

        return getStatus("erase");
    }

    bool getStatus(const char *who) {
        uint16_t flTemp[2];
        uint32_t status;
        if ( fCore.queue.send( readDataRegister(0) ) == false ||
             fCore.getData(flTemp, TAP_DO_READREGISTER, 4, 0) == false ){ // Status
            fCore.castMessage("Error: Unable to retrieve %s status", who);
            return false;
        }

        status = *(uint32_t *)flTemp;

        if ( status != 1 ) {
            fCore.castMessage("Error: %s flagged a fault", who);
            return false;
        }
        return true;
    }

public:
    explicit CPU32_genflash(bdmstuff &p)
        : requests(p), requests_cpu32(p), fCore(p) {
        driverBase = 0;
        flashBase = 0;
        bufferBase = 0;
        driverInited = false;
        clearFlashParams();
    }

    // Upload flash detection driver and detect flash
    bool detect(flashid_t &id, uint32_t destination, uint32_t flshbase = 0) {
        uint16_t idTemp[4];
        uint16_t status;
        bool retVal;

        clearFlashParams();
        driverInited = false;
        flashBase = flshbase;

        // Upload driver
        fCore.castMessage("Info: Uploading detect driver..");

        if ( fillDataBE4(destination, genDetect, sizeof(genDetect)) == false ) {
            fCore.castMessage("Error: Unable to upload driver");
            return false;
        }

        fCore.queue  = writeAddressRegister(0, flshbase);
        fCore.queue += writeSystemRegister(0, destination);
        fCore.queue += targetStart();

        if ( !fCore.queue.send() ) {
            fCore.castMessage("Error: detect - Unable to configure and/or start driver");
            return false;
        }

        fCore.castMessage("Info: Waiting..");

        do {
            retVal = fCore.getStatus(&status);
            sleep_ms(25);
        } while ( retVal && status == RET_TARGETRUNNING );

        if ( status != RET_TARGETSTOPPED ) {
            fCore.castMessage("Error: detect - Could not stop target");
            return false;
        }

        fCore.castMessage("Info: Retrieving flash info..");

        fCore.queue  = readDataRegister(4); // MID
        fCore.queue += readDataRegister(5); // DID / PID

        if ( fCore.queue.send() == false ||
             fCore.getData(&idTemp[0], TAP_DO_READREGISTER, 4, 0) == false || // MID
             fCore.getData(&idTemp[2], TAP_DO_READREGISTER, 4, 1) == false ){ // DID / PID
            fCore.castMessage("Error: Unable to retrieve flash information");
            return false;
        }

        fCore.castMessage("Info: MID    %04X", idTemp[0]);
        fCore.castMessage("Info: DID    %04X", idTemp[2]);
        fCore.castMessage("Info: base %06x", flshbase);

        if ( !getStatus("detect") ) 
            return false;

        id.MID = idTemp[0];
        id.DID = idTemp[2];

        flashKnown = true;

        return true;
    }

    // Upload and init flash / erase driver
    bool upload(const flashpart_t *part, uint32_t destination, uint32_t buffer, uint32_t chpCount = 1) {
        
        uint16_t status;
        bool retVal;

        if ( !flashKnown ) {
            fCore.castMessage("Error: You must detect flash before uploading the flash driver");
            return false;
        }

        if ( part == nullptr ) {
            fCore.castMessage("Error: Need flash specifications for this operation");
            return false;
        }

        chipCount = chpCount;
        driverBase = destination;
        bufferBase = buffer;
        driverInited = false;
        flashPart = part;

        // Upload driver
        fCore.castMessage("Info: Uploading flash driver..");

        if ( fillDataBE4(destination, genDriver, sizeof(genDriver)) == false ) {
            fCore.castMessage("Error: Unable to upload driver");
            return false;
        }

        fCore.queue  = writeDataRegister(0, 4);
        fCore.queue += writeDataRegister(7, part->type);
        fCore.queue += writeAddressRegister(0, flashBase);
        fCore.queue += writeSystemRegister(0, destination);
        fCore.queue += targetStart();

        if ( !fCore.queue.send() ) {
            fCore.castMessage("Error: Unable to configure and/or start driver");
            return false;
        }

        do {
            retVal = fCore.getStatus(&status);
            sleep_ms(25);
        } while ( retVal && status == RET_TARGETRUNNING );

        if ( status != RET_TARGETSTOPPED ) {
            fCore.castMessage("Error: Could not stop target");
            return false;
        }

        if ( !getStatus("init") )
            return false;

        driverInited = true;

        return true;
    }

    // Erase flash
    bool erase(uint32_t mask) {

        uint32_t Start = flashBase;

        if ( !driverInited ) {
            fCore.castMessage("Error: You must upload the flash driver before using this feature");
            return false;
        }

        if ( chipCount > 2 || chipCount == 0 || flashPart == nullptr || flashPart->count == 0 ) {
            fCore.castMessage("Error: Need valid partition parameters");
            return false;
        }

        // Perform bulk erase
        if ( mask == 0 || flashPart->count == 1 )
            return doBulkErase( flashBase, flashBase + (flashPart->partitions[ flashPart->count - 1 ] * chipCount) );

        // Sector erase
        for ( uint32_t i = 0; i < flashPart->count; i++ ) {

            if ( ((1 << i) & mask) != 0 ) {
                if ( !doSectorErase(Start, flashBase + (flashPart->partitions[ i ] * chipCount)) )
                    return false;
            }

            // Map stores the last address of every partition + 1
            Start = flashBase + (flashPart->partitions[ i ] * chipCount);
        }

        return true;
    }

    // Write flash
    // Couple of adapter quirks in CPU32 mode that must be known:
    // 1 - It won't adjust buffer size according to remaining length so request must be in multiples of that
    // 2 - There must never be less than 8 bytes per write iteration
    // 3 - Everything must be in multiples of 4
    bool write(uint32_t mask) {

        uint32_t Start = flashBase;
        uint32_t maskOffs = 0;
        memory_t memSpec = { opFlash };

        if ( !driverInited ) {
            fCore.castMessage("Error: You must upload the flash driver before using this feature");
            return false;
        }

        if ( chipCount > 2 || chipCount == 0 || flashPart == nullptr || flashPart->count == 0 ) {
            fCore.castMessage("Error: Need valid partition parameters");
            return false;
        }

        if ( flashPart->count > 32 ) {
            fCore.castMessage("Error: I don't know what to do with this many partitions!");
            return false;
        }

        // Write all
        if ( mask == 0 )
            mask = ~mask;

        // Driver needs to know where the buffer is located
        if ( fCore.queue.send(writeAddressRegister(1, bufferBase)) == false ) { 
            fCore.castMessage("Unable to set write start pointer");
            return false;
        }

        while ( maskOffs < flashPart->count ) {

            // Not set, loop over and check the next bit
            if ( ((1 << maskOffs) & mask) == 0 ) {
                maskOffs++;
                continue;
            }


            if ( maskOffs > 0 )
                Start = flashBase + (flashPart->partitions[ maskOffs - 1 ] * chipCount);

            maskOffs++;

            // Traverse the mask and figure out the largest continuous block
            while ( ((1 << maskOffs) & mask) != 0 && maskOffs < flashPart->count )
                maskOffs++;

            uint32_t Length = (flashBase + (flashPart->partitions[ maskOffs - 1 ] * chipCount)) - Start;
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



            fCore.castMessage("Info: Writing %06x - %06x..", Start, Start + Length - 1);

            // Assist function is VERY stupid on CPU32
            fCore.queue  = writeAddressRegister(0, Start);
            fCore.queue += writeDataRegister(1, bufSize / 2);
            if ( !fCore.queue.send() ) {
                fCore.castMessage("Error: Unable to update buffer size");
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

                core.castMessage("Warning: Untested runt feature caught remaining data");

                if ( (runtBytes & 7) != 0 ) {
                    core.castMessage("Error: Can't write in other than multiples of 8 (%u)", runtBytes);
                    return false;
                }

                fCore.castMessage("Info: Writing %06x - %06x..", Start + Length, Start + Length + runtBytes - 1);

                if ( fCore.queue.send(writeDataRegister(1, runtBytes)) == false ) { 
                    fCore.castMessage("Error: Unable to update buffer size");
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

class cpu32_utils
    : private CPU32_genmd5, private CPU32_genflash {

protected:
    CPU32_genflash &flash;
    CPU32_genmd5 &md5;

public:
    explicit cpu32_utils(bdmstuff &p)
        : requests(p), requests_cpu32(p),
            CPU32_genmd5(p), CPU32_genflash(p), flash(*this), md5(*this)
    {
        printf("cpu32_utils()\n");
    }
};

#endif

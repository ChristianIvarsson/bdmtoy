#ifndef __FLASH_CPU32_H__
#define __FLASH_CPU32_H__

#include "../../../../bdmstuff.h"
#include "../../../../requests_cpu32.h"

// Flash driver
#include "../../../drivers/CPU32/generic/cpu32_flash16.h"

class CPU32_genflash : public virtual CPU32_requests {
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

        core.castMessage("Info: Erasing %06x - %06x..", start, end - 1);
        // core.castMessage("Info: Driver base %08x", driverBase);

        core.queue  = writeDataRegister(0, 3);
        core.queue += writeAddressRegister(0, start);
        core.queue += writeAddressRegister(1, end);
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

        return getStatus("erase");
    }

    bool doSectorErase(uint32_t start, uint32_t end) {
        uint16_t status;
        bool retVal;

        core.castMessage("Info: Erasing %06x - %06x..", start, end - 1);
        // core.castMessage("Info: Driver base %08x", driverBase);

        core.queue  = writeDataRegister(0, 2);
        core.queue += writeAddressRegister(0, start);
        core.queue += writeAddressRegister(1, end);
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

        return getStatus("erase");
    }

    bool getStatus(const char *who) {
        uint16_t flTemp[2];
        uint32_t status;
        if ( core.queue.send( readDataRegister(0) ) == false ||
             core.getData(flTemp, TAP_DO_READREGISTER, 4, 0) == false ){ // Status
            core.castMessage("Error: Unable to retrieve %s status", who);
            return false;
        }

        status = *(uint32_t *)flTemp;

        if ( status != 1 ) {
            core.castMessage("Error: %s flagged a fault", who);
            return false;
        }
        return true;
    }

    // 8-bit
    // AA to 5555(base)
    // 55 to 2AAA(base)
    // 90 to 5555(base)

    // 2 x 8-bit
    // AAAA to AAAA(base)
    // 5555 to 5554(base)
    // 9090 to AAAA(base)

    // 16-bit
    // 00AA to AAAA(base)
    // 0055 to 5554(base)
    // 0090 to AAAA(base)
    bool sendOldcshool(uint32_t *id, uint32_t flshbase, eFlashWidth width, uint32_t chipCount) {
        uint32_t skipCount = 1;

        core.castMessage("Info: Detecting with oldschool commands..");

        switch ( width ) {
        case enWidth8: break;
        case enWidth16:
            skipCount = 2;
            break;
        case enWidth32:
            skipCount = 4;
            break;
        default:
            core.castMessage("Error: Don't know how to deal with this width");
            return false;
        }

        switch ( chipCount ) {
        case 1: break;
        case 2:
            skipCount <<= 1;
            break;
       case 4:
            skipCount <<= 2;
            break;
        default:
            core.castMessage("Error: Don't know how to deal with this number of chips");
            return false;
        }

        switch ( width ) {
        case enWidth8:
            for ( uint32_t i = 0; i < chipCount; i++ ) {

                if ( core.queue.send( writeMemory( flshbase + 32, 0x00000090, sizeByte ) ) == false ) {
                    core.castMessage("Error: Unable to send ID command");
                    return false;
                }

                sleep_ms( 10 );

                id[(i*2) + 0] = 0;
                id[(i*2) + 1] = 0;

                core.queue  = readMemory( flshbase + 0, sizeByte );
                core.queue += readMemory( flshbase + skipCount, sizeByte );
                if ( !core.queue.send() ||
                     !core.getData( (uint16_t*)&id[(i*2) + 0], TAP_DO_READMEMORY, sizeByte, 0 ) ||
                     !core.getData( (uint16_t*)&id[(i*2) + 1], TAP_DO_READMEMORY, sizeByte, 1 ) ){
                    core.castMessage("Error: Unable to retrieve ID data");
                    return false;
                }

                if ( core.queue.send( writeMemory( flshbase + 32, 0x000000FF, sizeByte ) ) == false ) {
                    core.castMessage("Error: Unable to send reset command");
                    return false;
                }

                flshbase += 1;
            }
            sleep_ms( 10 );
            return true;

        case enWidth16:
            for ( uint32_t i = 0; i < chipCount; i++ ) {

                if ( core.queue.send( writeMemory( flshbase + 32, 0x00000090, sizeWord ) ) == false ) {
                    core.castMessage("Error: Unable to send ID command");
                    return false;
                }

                sleep_ms( 10 );

                id[(i*2) + 0] = 0;
                id[(i*2) + 1] = 0;

                core.queue  = readMemory( flshbase + 0, sizeWord );
                core.queue += readMemory( flshbase + skipCount, sizeWord );
                if ( !core.queue.send() ||
                     !core.getData( (uint16_t*)&id[(i*2) + 0], TAP_DO_READMEMORY, sizeWord, 0 ) ||
                     !core.getData( (uint16_t*)&id[(i*2) + 1], TAP_DO_READMEMORY, sizeWord, 1 ) ){
                    core.castMessage("Error: Unable to retrieve ID data");
                    return false;
                }

                if ( core.queue.send( writeMemory( flshbase + 32, 0x000000FF, sizeWord ) ) == false ) {
                    core.castMessage("Error: Unable to send reset command");
                    return false;
                }

                flshbase += 2;
            }
            sleep_ms( 10 );
            return true;

        case enWidth32:
            for ( uint32_t i = 0; i < chipCount; i++ ) {

                if ( core.queue.send( writeMemory( flshbase + 32, 0x00000090, sizeDword ) ) == false ) {
                    core.castMessage("Error: Unable to send ID command");
                    return false;
                }

                sleep_ms( 10 );

                id[(i*2) + 0] = 0;
                id[(i*2) + 1] = 0;

                core.queue  = readMemory( flshbase + 0, sizeDword );
                core.queue += readMemory( flshbase + skipCount, sizeDword );
                if ( !core.queue.send() ||
                     !core.getData( (uint16_t*)&id[(i*2) + 0], TAP_DO_READMEMORY, sizeDword, 0 ) ||
                     !core.getData( (uint16_t*)&id[(i*2) + 1], TAP_DO_READMEMORY, sizeDword, 1 ) ){
                    core.castMessage("Error: Unable to retrieve ID data");
                    return false;
                }

                if ( core.queue.send( writeMemory( flshbase + 32, 0x000000FF, sizeDword ) ) == false ) {
                    core.castMessage("Error: Unable to send reset command");
                    return false;
                }

                flshbase += 4;
            }
            sleep_ms( 10 );
            return true;

        default:
            core.castMessage("Error: Don't know how to deal with this width");
            return false;
        }

        return false;
    }

    bool sendJEDEC(uint32_t *id, uint32_t flshbase, eFlashWidth width, uint32_t chipCount) {

        uint32_t addressA = 0x5555;
        uint32_t addressB = 0x2AAA;
        uint32_t skipCount = 1;

        core.castMessage("Info: Detecting with JEDEC commands..");

        switch ( width ) {
        case enWidth8: break;
        case enWidth16:
            skipCount = 2;
            addressA <<= 1;
            addressB <<= 1;
            break;
        case enWidth32:
            skipCount = 4;
            addressA <<= 2;
            addressB <<= 2;
            break;
        default:
            core.castMessage("Error: Don't know how to deal with this width");
            return false;
        }

        switch ( chipCount ) {
        case 1: break;
        case 2:
            skipCount <<= 1;
            addressA <<= 1;
            addressB <<= 1;
            break;
       case 4:
            skipCount <<= 2;
            addressA <<= 2;
            addressB <<= 2;
            break;
        default:
            core.castMessage("Error: Don't know how to deal with this number of chips");
            return false;
        }

        addressA += flshbase;
        addressB += flshbase;

        switch ( width ) {
        case enWidth8:
            for ( uint32_t i = 0; i < chipCount; i++ ) {
                core.queue  = writeMemory( addressA, 0x000000AA, sizeByte );
                core.queue += writeMemory( addressB, 0x00000055, sizeByte );
                core.queue += writeMemory( addressA, 0x00000090, sizeByte );
                if ( core.queue.send() == false ) {
                    core.castMessage("Error: Unable to send ID command");
                    return false;
                }

                sleep_ms( 10 );

                id[(i*2) + 0] = 0;
                id[(i*2) + 1] = 0;

                core.queue  = readMemory( flshbase + 0, sizeByte );
                core.queue += readMemory( flshbase + skipCount, sizeByte );
                if ( !core.queue.send() ||
                     !core.getData( (uint16_t*)&id[(i*2) + 0], TAP_DO_READMEMORY, sizeByte, 0 ) ||
                     !core.getData( (uint16_t*)&id[(i*2) + 1], TAP_DO_READMEMORY, sizeByte, 1 ) ){
                    core.castMessage("Error: Unable to retrieve ID data");
                    return false;
                }

                core.queue  = writeMemory( addressA, 0x000000AA, sizeByte );
                core.queue += writeMemory( addressB, 0x00000055, sizeByte );
                core.queue += writeMemory( addressA, 0x000000F0, sizeByte );
                if ( core.queue.send() == false ) {
                    core.castMessage("Error: Unable to send reset command");
                    return false;
                }

                addressA += 1;
                addressB += 1;
                flshbase += 1;
            }
            sleep_ms( 10 );
            return true;

        case enWidth16:
            for ( uint32_t i = 0; i < chipCount; i++ ) {
                core.queue  = writeMemory( addressA, 0x000000AA, sizeWord );
                core.queue += writeMemory( addressB, 0x00000055, sizeWord );
                core.queue += writeMemory( addressA, 0x00000090, sizeWord );
                if ( core.queue.send() == false ) {
                    core.castMessage("Error: Unable to send ID command");
                    return false;
                }

                sleep_ms( 10 );

                id[(i*2) + 0] = 0;
                id[(i*2) + 1] = 0;

                core.queue  = readMemory( flshbase + 0, sizeWord );
                core.queue += readMemory( flshbase + skipCount, sizeWord );
                if ( !core.queue.send() ||
                     !core.getData( (uint16_t*)&id[(i*2) + 0], TAP_DO_READMEMORY, sizeWord, 0 ) ||
                     !core.getData( (uint16_t*)&id[(i*2) + 1], TAP_DO_READMEMORY, sizeWord, 1 ) ){
                    core.castMessage("Error: Unable to retrieve ID data");
                    return false;
                }

                core.queue  = writeMemory( addressA, 0x000000AA, sizeWord );
                core.queue += writeMemory( addressB, 0x00000055, sizeWord );
                core.queue += writeMemory( addressA, 0x000000F0, sizeWord );
                if ( core.queue.send() == false ) {
                    core.castMessage("Error: Unable to send reset command");
                    return false;
                }

                addressA += 2;
                addressB += 2;
                flshbase += 2;
            }
            sleep_ms( 10 );
            return true;

        case enWidth32:
            for ( uint32_t i = 0; i < chipCount; i++ ) {
                core.queue  = writeMemory( addressA, 0x000000AA, sizeDword );
                core.queue += writeMemory( addressB, 0x00000055, sizeDword );
                core.queue += writeMemory( addressA, 0x00000090, sizeDword );
                if ( core.queue.send() == false ) {
                    core.castMessage("Error: Unable to send ID command");
                    return false;
                }

                sleep_ms( 10 );

                id[(i*2) + 0] = 0;
                id[(i*2) + 1] = 0;

                core.queue  = readMemory( flshbase + 0, sizeDword );
                core.queue += readMemory( flshbase + skipCount, sizeDword );
                if ( !core.queue.send() ||
                     !core.getData( (uint16_t*)&id[(i*2) + 0], TAP_DO_READMEMORY, sizeDword, 0 ) ||
                     !core.getData( (uint16_t*)&id[(i*2) + 1], TAP_DO_READMEMORY, sizeDword, 1 ) ){
                    core.castMessage("Error: Unable to retrieve ID data");
                    return false;
                }

                core.queue  = writeMemory( addressA, 0x000000AA, sizeDword );
                core.queue += writeMemory( addressB, 0x00000055, sizeDword );
                core.queue += writeMemory( addressA, 0x000000F0, sizeDword );
                if ( core.queue.send() == false ) {
                    core.castMessage("Error: Unable to send reset command");
                    return false;
                }

                addressA += 4;
                addressB += 4;
                flshbase += 4;
            }
            sleep_ms( 10 );
            return true;

        default:
            core.castMessage("Error: Don't know how to deal with this width");
            return false;
        }

        return false;
    }

public:
    explicit CPU32_genflash(bdmstuff &p)
        : requests(p), CPU32_requests(p) {
        driverBase = 0;
        flashBase = 0;
        bufferBase = 0;
        driverInited = false;
        clearFlashParams();
    }

    bool detect_mk2(flashid_t &id, uint32_t flshbase, eFlashWidth width, uint32_t chipCount = 1) {
        uint32_t idTemp[8];
        const flashpart_t *part;
        bool allMatched = true;

        clearFlashParams();
        driverInited = false;
        flashBase = flshbase;

        if ( chipCount == 0 || chipCount > 2 ) {
            core.castMessage("Error: Don't know how to deal with this chip count");
            return false;
        }

        core.castMessage("Info: base %06x", flshbase);

        // Be silent the first time
        if ( !sendOldcshool( idTemp, flshbase, width, chipCount) )
            return false;

        for ( uint32_t i = 0; i < chipCount; i++ ) {
            if ( parthelper::getMap(idTemp[(i*2) + 0], idTemp[(i*2) + 1], width ) == nullptr )
                allMatched = false;
        }

        if ( allMatched == true ) {
            for ( uint32_t i = 0; i < chipCount; i++ ) {
                part = parthelper::getMap(idTemp[(i*2) + 0], idTemp[(i*2) + 1], width );
                core.castMessage("Info: Chip %u MID    %04X (%s)", i, idTemp[(i*2) + 0], parthelper::getManufacturerName( idTemp[(i*2) + 0] ));
                if ( part == nullptr || part->pName == nullptr || part->pName[0] == 0 ) {
                    core.castMessage("Info: Chip %u DID    %04X (Unknown)", i, idTemp[(i*2) + 1]);
                    if ( part == nullptr ) {
                        core.castMessage("Error: You should definitely not see this message!", i, idTemp[(i*2) + 1], part->pName);
                        return false;
                    }
                } else {
                    core.castMessage("Info: Chip %u DID    %04X (%s)", i, idTemp[(i*2) + 1], part->pName);
                }

                if ( i > 0 &&
                        (idTemp[(i*2) + 0] != idTemp[(i*2) - 2] ||
                         idTemp[(i*2) + 1] != idTemp[(i*2) - 1])) {
                    core.castMessage("Warning: Part numbers doesn't match!");
                }
            }
        } else {

            allMatched = true;

            core.castMessage("Info: failed");

            if ( !sendJEDEC( idTemp, flshbase, width, chipCount) )
                return false;

            for ( uint32_t i = 0; i < chipCount; i++ ) {

                part = parthelper::getMap(idTemp[(i*2) + 0], idTemp[(i*2) + 1], width );

                core.castMessage("Info: Chip %u MID    %04X (%s)", i, idTemp[(i*2) + 0], parthelper::getManufacturerName( idTemp[(i*2) + 0] ));
                if ( part == nullptr || part->pName == nullptr || part->pName[0] == 0 ) {
                    core.castMessage("Info: Chip %u DID    %04X (Unknown)", i, idTemp[(i*2) + 1]);
                    if ( part == nullptr )
                        allMatched = false;
                } else {
                    core.castMessage("Info: Chip %u DID    %04X (%s)", i, idTemp[(i*2) + 1], part->pName);
                }

                if ( i > 0 &&
                        (idTemp[(i*2) + 0] != idTemp[(i*2) - 2] ||
                         idTemp[(i*2) + 1] != idTemp[(i*2) - 1])) {
                    core.castMessage("Warning: Part numbers doesn't match!");
                }
            }
        }

        if ( !allMatched ) {
            core.castMessage("Error: Unable to detect");
            return false;
        }

        // Sample first chip ID
        id.MID = idTemp[0];
        id.DID = idTemp[1];
        flashKnown = true;

        return true;
    }

    // Upload and init flash / erase driver
    bool upload(const flashpart_t *part, uint32_t destination, uint32_t buffer, uint32_t chpCount = 1) {
        
        uint16_t status;
        bool retVal;

        if ( !flashKnown ) {
            core.castMessage("Error: You must detect flash before uploading the flash driver");
            return false;
        }

        if ( part == nullptr ) {
            core.castMessage("Error: Need flash specifications for this operation");
            return false;
        }

        chipCount = chpCount;
        driverBase = destination;
        bufferBase = buffer;
        driverInited = false;
        flashPart = part;

        // Upload driver
        core.castMessage("Info: Uploading flash driver..");

        if ( fillDataBE4(destination, CPU32_flashdriver16, sizeof(CPU32_flashdriver16)) == false ) {
            core.castMessage("Error: Unable to upload driver");
            return false;
        }

        core.queue  = writeDataRegister(0, 4);
        core.queue += writeDataRegister(7, part->type);
        core.queue += writeAddressRegister(0, flashBase);
        core.queue += writeSystemRegister(0, destination);
        core.queue += targetStart();

        if ( !core.queue.send() ) {
            core.castMessage("Error: Unable to configure and/or start driver");
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

        if ( !getStatus("init") )
            return false;

        driverInited = true;

        return true;
    }

    // Erase flash
    bool erase(uint32_t mask) {

        uint32_t Start = flashBase;
        uint32_t nCount = 0;
        uint32_t totCount = 0;

        if ( !driverInited ) {
            core.castMessage("Error: You must upload the flash driver before using this feature");
            return false;
        }

        if ( chipCount > 2 || chipCount == 0 || flashPart == nullptr || flashPart->count == 0 ) {
            core.castMessage("Error: Need valid partition parameters");
            return false;
        }

        // Perform bulk erase
        if ( mask == 0 || flashPart->count == 1 )
            return doBulkErase( flashBase, flashBase + (flashPart->partitions[ flashPart->count - 1 ] * chipCount) );

        core.updateProgress( 0 );

        // Determine number of partitions so that progress can be updated
        for ( uint32_t i = 0; i < flashPart->count; i++ ) {
            if ( ((1 << i) & mask) != 0 )
                totCount++;
        }

        // Sector erase
        for ( uint32_t i = 0; i < flashPart->count; i++ ) {

            if ( ((1 << i) & mask) != 0 ) {
                if ( !doSectorErase(Start, flashBase + (flashPart->partitions[ i ] * chipCount)) )
                    return false;
                core.updateProgress((int32_t)(float)100.0 * ++nCount / totCount);
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
        size_t   totLen = 0;

        if ( !driverInited ) {
            core.castMessage("Error: You must upload the flash driver before using this feature");
            return false;
        }

        if ( chipCount > 2 || chipCount == 0 || flashPart == nullptr || flashPart->count == 0 ) {
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
                Start = flashBase + (flashPart->partitions[ maskOffs - 1 ] * chipCount);
            totLen += ((flashBase + (flashPart->partitions[ maskOffs++ ] * chipCount)) - Start);
        }

        // Set total len
        core.pagedProgress( true, totLen );

        // Start over and do it for real this time
        Start = flashBase;
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

                core.castMessage("Warning: Untested runt feature caught remaining data");

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

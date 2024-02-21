#ifndef __UTILS_CPU32_H__
#define __UTILS_CPU32_H__

// Target-specific driver.. stuff
#include "../../../bdmstuff.h"
#include "../../../requests_cpu32.h"
#include "../../../utils/crypto.h"

#include "../utils_shared.h"
#include "../../partition_helper.h"

// Target drivers
#include "../../drivers/CPU32/generic/cpu32_flash16.h"
#include "../../drivers/CPU32/generic/cpu32_md5.h"

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

    bool upload(const uint8_t *data, size_t nBytes, uint32_t address, bool silent = false) {
        driverBase = address;

        // Upload driver
        if ( !silent )
            mdCore.castMessage("Info: Uploading md5 hash driver..");

        if ( fillDataBE4(address, data, nBytes) == false ) {
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

        fCore.castMessage("Info: Detecting with oldschool commands..");

        switch ( width ) {
        case enWidth8: break;
        case enWidth16:
            skipCount = 2;
            break;
        case enWidth32:
            skipCount = 4;
            break;
        default:
            fCore.castMessage("Error: Don't know how to deal with this width");
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
            fCore.castMessage("Error: Don't know how to deal with this number of chips");
            return false;
        }

        switch ( width ) {
        case enWidth8:
            for ( uint32_t i = 0; i < chipCount; i++ ) {

                if ( fCore.queue.send( writeMemory( flshbase + 32, 0x00000090, sizeByte ) ) == false ) {
                    fCore.castMessage("Error: Unable to send ID command");
                    return false;
                }

                sleep_ms( 10 );

                id[(i*2) + 0] = 0;
                id[(i*2) + 1] = 0;

                fCore.queue  = readMemory( flshbase + 0, sizeByte );
                fCore.queue += readMemory( flshbase + skipCount, sizeByte );
                if ( !fCore.queue.send() ||
                     !fCore.getData( (uint16_t*)&id[(i*2) + 0], TAP_DO_READMEMORY, sizeByte, 0 ) ||
                     !fCore.getData( (uint16_t*)&id[(i*2) + 1], TAP_DO_READMEMORY, sizeByte, 1 ) ){
                    core.castMessage("Error: Unable to retrieve ID data");
                    return false;
                }

                if ( fCore.queue.send( writeMemory( flshbase + 32, 0x000000FF, sizeByte ) ) == false ) {
                    fCore.castMessage("Error: Unable to send reset command");
                    return false;
                }

                flshbase += 1;
            }
            sleep_ms( 10 );
            return true;

        case enWidth16:
            for ( uint32_t i = 0; i < chipCount; i++ ) {

                if ( fCore.queue.send( writeMemory( flshbase + 32, 0x00000090, sizeWord ) ) == false ) {
                    fCore.castMessage("Error: Unable to send ID command");
                    return false;
                }

                sleep_ms( 10 );

                id[(i*2) + 0] = 0;
                id[(i*2) + 1] = 0;

                fCore.queue  = readMemory( flshbase + 0, sizeWord );
                fCore.queue += readMemory( flshbase + skipCount, sizeWord );
                if ( !fCore.queue.send() ||
                     !fCore.getData( (uint16_t*)&id[(i*2) + 0], TAP_DO_READMEMORY, sizeWord, 0 ) ||
                     !fCore.getData( (uint16_t*)&id[(i*2) + 1], TAP_DO_READMEMORY, sizeWord, 1 ) ){
                    core.castMessage("Error: Unable to retrieve ID data");
                    return false;
                }

                if ( fCore.queue.send( writeMemory( flshbase + 32, 0x000000FF, sizeWord ) ) == false ) {
                    fCore.castMessage("Error: Unable to send reset command");
                    return false;
                }

                flshbase += 2;
            }
            sleep_ms( 10 );
            return true;

        case enWidth32:
            for ( uint32_t i = 0; i < chipCount; i++ ) {

                if ( fCore.queue.send( writeMemory( flshbase + 32, 0x00000090, sizeDword ) ) == false ) {
                    fCore.castMessage("Error: Unable to send ID command");
                    return false;
                }

                sleep_ms( 10 );

                id[(i*2) + 0] = 0;
                id[(i*2) + 1] = 0;

                fCore.queue  = readMemory( flshbase + 0, sizeDword );
                fCore.queue += readMemory( flshbase + skipCount, sizeDword );
                if ( !fCore.queue.send() ||
                     !fCore.getData( (uint16_t*)&id[(i*2) + 0], TAP_DO_READMEMORY, sizeDword, 0 ) ||
                     !fCore.getData( (uint16_t*)&id[(i*2) + 1], TAP_DO_READMEMORY, sizeDword, 1 ) ){
                    core.castMessage("Error: Unable to retrieve ID data");
                    return false;
                }

                if ( fCore.queue.send( writeMemory( flshbase + 32, 0x000000FF, sizeDword ) ) == false ) {
                    fCore.castMessage("Error: Unable to send reset command");
                    return false;
                }

                flshbase += 4;
            }
            sleep_ms( 10 );
            return true;

        default:
            fCore.castMessage("Error: Don't know how to deal with this width");
            return false;
        }

        return false;
    }

    bool sendJEDEC(uint32_t *id, uint32_t flshbase, eFlashWidth width, uint32_t chipCount) {

        uint32_t addressA = 0x5555;
        uint32_t addressB = 0x2AAA;
        uint32_t skipCount = 1;

        fCore.castMessage("Info: Detecting with JEDEC commands..");

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
            fCore.castMessage("Error: Don't know how to deal with this width");
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
            fCore.castMessage("Error: Don't know how to deal with this number of chips");
            return false;
        }

        addressA += flshbase;
        addressB += flshbase;

        switch ( width ) {
        case enWidth8:
            for ( uint32_t i = 0; i < chipCount; i++ ) {
                fCore.queue  = writeMemory( addressA, 0x000000AA, sizeByte );
                fCore.queue += writeMemory( addressB, 0x00000055, sizeByte );
                fCore.queue += writeMemory( addressA, 0x00000090, sizeByte );
                if ( fCore.queue.send() == false ) {
                    fCore.castMessage("Error: Unable to send ID command");
                    return false;
                }

                sleep_ms( 10 );

                id[(i*2) + 0] = 0;
                id[(i*2) + 1] = 0;

                fCore.queue  = readMemory( flshbase + 0, sizeByte );
                fCore.queue += readMemory( flshbase + skipCount, sizeByte );
                if ( !fCore.queue.send() ||
                     !fCore.getData( (uint16_t*)&id[(i*2) + 0], TAP_DO_READMEMORY, sizeByte, 0 ) ||
                     !fCore.getData( (uint16_t*)&id[(i*2) + 1], TAP_DO_READMEMORY, sizeByte, 1 ) ){
                    core.castMessage("Error: Unable to retrieve ID data");
                    return false;
                }

                fCore.queue  = writeMemory( addressA, 0x000000AA, sizeByte );
                fCore.queue += writeMemory( addressB, 0x00000055, sizeByte );
                fCore.queue += writeMemory( addressA, 0x000000F0, sizeByte );
                if ( fCore.queue.send() == false ) {
                    fCore.castMessage("Error: Unable to send reset command");
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
                fCore.queue  = writeMemory( addressA, 0x000000AA, sizeWord );
                fCore.queue += writeMemory( addressB, 0x00000055, sizeWord );
                fCore.queue += writeMemory( addressA, 0x00000090, sizeWord );
                if ( fCore.queue.send() == false ) {
                    fCore.castMessage("Error: Unable to send ID command");
                    return false;
                }

                sleep_ms( 10 );

                id[(i*2) + 0] = 0;
                id[(i*2) + 1] = 0;

                fCore.queue  = readMemory( flshbase + 0, sizeWord );
                fCore.queue += readMemory( flshbase + skipCount, sizeWord );
                if ( !fCore.queue.send() ||
                     !fCore.getData( (uint16_t*)&id[(i*2) + 0], TAP_DO_READMEMORY, sizeWord, 0 ) ||
                     !fCore.getData( (uint16_t*)&id[(i*2) + 1], TAP_DO_READMEMORY, sizeWord, 1 ) ){
                    core.castMessage("Error: Unable to retrieve ID data");
                    return false;
                }

                fCore.queue  = writeMemory( addressA, 0x000000AA, sizeWord );
                fCore.queue += writeMemory( addressB, 0x00000055, sizeWord );
                fCore.queue += writeMemory( addressA, 0x000000F0, sizeWord );
                if ( fCore.queue.send() == false ) {
                    fCore.castMessage("Error: Unable to send reset command");
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
                fCore.queue  = writeMemory( addressA, 0x000000AA, sizeDword );
                fCore.queue += writeMemory( addressB, 0x00000055, sizeDword );
                fCore.queue += writeMemory( addressA, 0x00000090, sizeDword );
                if ( fCore.queue.send() == false ) {
                    fCore.castMessage("Error: Unable to send ID command");
                    return false;
                }

                sleep_ms( 10 );

                id[(i*2) + 0] = 0;
                id[(i*2) + 1] = 0;

                fCore.queue  = readMemory( flshbase + 0, sizeDword );
                fCore.queue += readMemory( flshbase + skipCount, sizeDword );
                if ( !fCore.queue.send() ||
                     !fCore.getData( (uint16_t*)&id[(i*2) + 0], TAP_DO_READMEMORY, sizeDword, 0 ) ||
                     !fCore.getData( (uint16_t*)&id[(i*2) + 1], TAP_DO_READMEMORY, sizeDword, 1 ) ){
                    core.castMessage("Error: Unable to retrieve ID data");
                    return false;
                }

                fCore.queue  = writeMemory( addressA, 0x000000AA, sizeDword );
                fCore.queue += writeMemory( addressB, 0x00000055, sizeDword );
                fCore.queue += writeMemory( addressA, 0x000000F0, sizeDword );
                if ( fCore.queue.send() == false ) {
                    fCore.castMessage("Error: Unable to send reset command");
                    return false;
                }

                addressA += 4;
                addressB += 4;
                flshbase += 4;
            }
            sleep_ms( 10 );
            return true;

        default:
            fCore.castMessage("Error: Don't know how to deal with this width");
            return false;
        }

        return false;
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

    bool detect_mk2(flashid_t &id, uint32_t flshbase, eFlashWidth width, uint32_t chipCount = 1) {
        uint32_t idTemp[8];
        const flashpart_t *part;
        bool allMatched = true;

        clearFlashParams();
        driverInited = false;
        flashBase = flshbase;

        if ( chipCount == 0 || chipCount > 2 ) {
            fCore.castMessage("Error: Don't know how to deal with this chip count");
            return false;
        }

        fCore.castMessage("Info: base %06x", flshbase);

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
                fCore.castMessage("Info: Chip %u MID    %04X (%s)", i, idTemp[(i*2) + 0], parthelper::getManufacturerName( idTemp[(i*2) + 0] ));
                if ( part == nullptr || part->pName == nullptr || part->pName[0] == 0 ) {
                    fCore.castMessage("Info: Chip %u DID    %04X (Unknown)", i, idTemp[(i*2) + 1]);
                    if ( part == nullptr ) {
                        fCore.castMessage("Error: You should definitely not see this message!", i, idTemp[(i*2) + 1], part->pName);
                        return false;
                    }
                } else {
                    fCore.castMessage("Info: Chip %u DID    %04X (%s)", i, idTemp[(i*2) + 1], part->pName);
                }

                if ( i > 0 &&
                        (idTemp[(i*2) + 0] != idTemp[(i*2) - 2] ||
                         idTemp[(i*2) + 1] != idTemp[(i*2) - 1])) {
                    fCore.castMessage("Warning: Part numbers doesn't match!");
                }
            }
        } else {

            allMatched = true;

            fCore.castMessage("Info: failed");

            if ( !sendJEDEC( idTemp, flshbase, width, chipCount) )
                return false;

            for ( uint32_t i = 0; i < chipCount; i++ ) {

                part = parthelper::getMap(idTemp[(i*2) + 0], idTemp[(i*2) + 1], width );

                fCore.castMessage("Info: Chip %u MID    %04X (%s)", i, idTemp[(i*2) + 0], parthelper::getManufacturerName( idTemp[(i*2) + 0] ));
                if ( part == nullptr || part->pName == nullptr || part->pName[0] == 0 ) {
                    fCore.castMessage("Info: Chip %u DID    %04X (Unknown)", i, idTemp[(i*2) + 1]);
                    if ( part == nullptr )
                        allMatched = false;
                } else {
                    fCore.castMessage("Info: Chip %u DID    %04X (%s)", i, idTemp[(i*2) + 1], part->pName);
                }

                if ( i > 0 &&
                        (idTemp[(i*2) + 0] != idTemp[(i*2) - 2] ||
                         idTemp[(i*2) + 1] != idTemp[(i*2) - 1])) {
                    fCore.castMessage("Warning: Part numbers doesn't match!");
                }
            }
        }

        if ( !allMatched ) {
            fCore.castMessage("Error: Unable to detect");
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

        if ( fillDataBE4(destination, CPU32_flashdriver16, sizeof(CPU32_flashdriver16)) == false ) {
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
        uint32_t nCount = 0;
        uint32_t totCount = 0;

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

        fCore.updateProgress( 0 );

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
                fCore.updateProgress((int32_t)(float)100.0 * ++nCount / totCount);
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
        fCore.pagedProgress( true, totLen );

        // Start over and do it for real this time
        Start = flashBase;
        maskOffs = 0;

        fCore.castMessage("Info: Total length %u bytes", totLen);

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

    bool genericFlash(
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
};

#endif

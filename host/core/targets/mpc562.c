#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "targets.h"
#include "../core.h"
#include "../core_worker.h"

#include "drivers/mpc562test.h" // Flash driver
#include "drivers/c39eeprom.h"  // EEPROM driver
#include "drivers/ppcmd5.h"     // MD5

// Data pins are scrambled...
static const uint8_t fpin[] = {
    30, 29, 27,  5, 25,  4, 23,  6, 21,  8, 19, 18, 17, 16, 12, 13,
    11, 15, 10, 14,  9, 20,  7, 22,  2, 24,  1, 26, 31, 28,  0,  3
};

static uint32_t unscramble16c39(const uint32_t data)
{
    uint32_t outdata = 0;
    uint8_t  i;

    for (i = 0; i < 32; i++)
        outdata |= (data & (1 << i)) ? 1 << fpin[i] : 0;

    return outdata;
}

static uint32_t scramble16c39(const uint32_t data)
{
    uint32_t outdata = 0;
    uint8_t  i;

    for (i = 0; i < 32; i++) {
        outdata |= (data & (1 << fpin[i])) ? 1 << i : 0;
    }

    return outdata;
}

uint32_t edc_SendAddressAndLen(uint32_t Address, uint32_t Len)
{
    wrk_newQueue( TAP_WriteRegister(5, 4, Address) );
    wrk_queueReq( TAP_WriteRegister(6, 4, Len) );
    return wrk_sendQueue();
}

static uint32_t PPC_PrintGPRSum()
{
    uint16_t *ptr;
    uint32_t  i, tempdata[4];
    
    for (i = 0; i < 32; i++) {
        ptr = wrk_requestData( TAP_ReadRegister((uint16_t)i, 4) );
        if (!ptr) {
            core_castText("Could not read test register r24");
            return busyOK();
        }
        tempdata[i&3] = *(uint32_t*) &ptr[2];

        if ((i&3)==3) {
            core_castText("%08X %08X %08X %08X   (%2u - %2u)",
                                        tempdata[0], tempdata[1],
                                        tempdata[2], tempdata[3],
                                        i-3        , i);
        }
    }
    return RET_OK;
}

static uint32_t PPC_UploadMD5(uint32_t Location)
{
    uint32_t retval;

    core_castText("Uploading md5 module..");
    // It should be BE4 but the words are already swapped due to using DMA on the adapter
    retval = TAP_FillDataBE2(Location, sizeof(ppcmd5_bin), &ppcmd5_bin);
    if (retval != RET_OK) core_castText("Fill failed!");

    return retval;
}

static uint32_t *PPC_ReqRemoteMD5(uint32_t driverLoc, uint32_t Start, uint32_t Length)
{
    uint32_t mKeys[4],i,retval;
    uint16_t *ptr;

    wrk_newQueue( TAP_WriteRegister(3      , 4, Start    ) );
    wrk_queueReq( TAP_WriteRegister(4      , 4, Length   ) );
    wrk_queueReq( TAP_WriteRegister(32 + 26, 4, driverLoc) );
    wrk_queueReq( TAP_TargetStart() );
    retval = wrk_sendQueue();
    if (retval != RET_OK) {
        core_castText("Remote md5 failed");
        return 0;
    }

    // Wait for target to stop
    do{ retval = TAP_TargetStatus();
    }while (retval == RET_TARGETRUNNING);
    if (retval != RET_TARGETSTOPPED) {
        core_castText("Failed to stop target");
        return 0;
    }

    for (i = 0; i < 4; i++) {
        ptr = wrk_requestData( TAP_ReadRegister(6 + (uint16_t)i, 4) );
        if (!ptr) {
            core_castText("Remote md5 failed");
            return 0;
        }
        mKeys[i] = *(uint32_t *) &ptr[2];
    }

    return utl_md5FinalTarget(&mKeys[0], Length);
}

static uint32_t EDC_DriverDemand(uint32_t what)
{
    uint32_t retval;
    uint16_t *ptr;

    wrk_newQueue( TAP_WriteRegister(3      , 4, what    ) );
    wrk_queueReq( TAP_WriteRegister(32 + 26, 4, 0x3F8000) );
    wrk_queueReq( TAP_TargetStart() );
    retval = wrk_sendQueue();
    if (retval != RET_OK) {
        core_castText("Failed to start target");
        return retval;
    }

    // Wait for target to stop
    do{ retval = TAP_TargetStatus();
    }while (retval == RET_TARGETRUNNING);
    if (retval != RET_TARGETSTOPPED) {
        core_castText("Failed to stop target");
        return retval;
    }
    
    ptr = wrk_requestData( TAP_ReadRegister(3, 4) );
    if (!ptr) {
        core_castText("Could not read status from r3");
        return busyOK();
    }

    if (*(uint32_t *)&ptr[2] != 1){
        core_castText("Driver fail on: %u", what);
        
        ptr = wrk_requestData( TAP_ReadRegister(4, 4) );
        if (!ptr) {
            core_castText("Could not fetch reason");
            return busyOK();
        }
        core_castText("Reason        : %x", *(uint32_t*)&ptr[2]);
        return 0xFFFF;
    }

    return RET_OK;
}

static uint32_t PrintFlashSignature()
{
    uint32_t retval, Signature1, Signature2;
    uint16_t *ptr;

    core_castText("Checking flash signatures..");

    retval = wrk_sendOneshot( TAP_WriteDword(0, scramble16c39(0x90) ) );
    if (retval != RET_OK) {
        core_castText("Failed to request signature");
        return retval;
    }
    
    // Read first signature
    ptr = wrk_requestData( TAP_ReadDword(0) );
    if (!ptr) {
        core_castText("Could not read signature");
        return busyOK();
    }
    Signature1 = unscramble16c39(*(uint32_t*) &ptr[2]);
    core_castText("Signature 1: %04X (%08X)", Signature1,*(uint32_t*) &ptr[2]);

    // Read second signature
    ptr = wrk_requestData( TAP_ReadDword(4) );
    if (!ptr) {
        core_castText("Could not read signature");
        return busyOK();
    }
    Signature2 = unscramble16c39(*(uint32_t*) &ptr[2]);
    core_castText("Signature 2: %04X (%08X)", Signature2,*(uint32_t*) &ptr[2]);

    // Return Flash to regular operation
    retval = wrk_sendOneshot( TAP_WriteDword(0, scramble16c39(0xFF) ) );
    if (retval != RET_OK) {
        core_castText("Failed to return flash to read mode");
        return retval;
    }

    if (Signature1 != 0x0020 || Signature2 != 0x8835) {
        core_castText("Unexpected flash. Aborting");
        return 0xFFFF;
    }
    else
        core_castText("Flash model and manufacturer known");

    return RET_OK;
}

uint32_t initEDC16C39()
{
    TAP_Config_host_t config;
    uint32_t retval, SCCR, PLPRCR;
    uint16_t SYPCR;
    uint16_t *ptr;
    uint32_t mf;
    uint32_t divf;

    config.Type           = TAP_IO_BDMNEW;
    config.Frequency      = 1000000;
    config.cfgmask.Endian = TAP_BIGENDIAN;

    wrk_ResetFault();

    wrk_newQueue( TAP_SetInterface(config) );
    wrk_queueReq( TAP_TargetReset() );
    retval = wrk_sendQueue();
    if (retval != RET_OK) return retval;

    ////////////////////////////////
    // Map everything from address 0
    ptr = wrk_requestData( TAP_ReadRegister(32 + 638, 4) );
    if (!ptr) {
        core_castText("Could not read IMMR");
        return busyOK();
    }

    core_castText("Partnumber: %04X", ptr[3]);
    if (ptr[3] != 0x3530) {
        core_castText("This part number doesn't look right. Aborting");
        return 0xFFFF;
    }

    // Mask off ISB (28:30) and FLEN (20)
    retval = *(uint32_t*) &ptr[2];
    retval &= ~(BIT_REVERSE(32, 30) | BIT_REVERSE(32, 29) | BIT_REVERSE(32, 28) | BIT_REVERSE(32, 20));

    retval = wrk_sendOneshot( TAP_WriteRegister(32 + 638, 4, retval) );
    if (retval != RET_OK) {
        core_castText("Failed to configure IMMR");
        return retval;
    }

    ////////////////////
    // Byyyye, watchdog!
    ptr = wrk_requestData( TAP_ReadWord(0x2FC006) );
    if (!ptr) {
        core_castText("Could not read sypcr");
        return busyOK();
    }

    SYPCR = ptr[2];
    SYPCR |= 1 << 3; // Set SWF
    SYPCR &= 0xFFFB; // Negate SWE

    retval = wrk_sendOneshot( TAP_WriteWord(0x2FC006, SYPCR) );
    if (retval != RET_OK) return retval;

    //////////////////////////
    // Poke around with clocks
    // SCCR
    ptr = wrk_requestData( TAP_ReadDword(0x2FC280) );
    if (!ptr) {
        core_castText("Could not read SCCR");
        return busyOK();
    }
    SCCR = *(uint32_t*)&ptr[2];

    // PLPRCR
    ptr = wrk_requestData( TAP_ReadDword(0x2FC284) );
    if (!ptr) {
        core_castText("Could not read PLPRCR");
        return busyOK();
    }
    PLPRCR = *(uint32_t*)&ptr[2];

    // core_castText("SCCR  : %08X", SCCR);
    // core_castText("PLPRCR: %08X", PLPRCR);
    
    // Negate MFPDL, LPML, LME (locks and limp)
    SCCR &= ~(BIT_REVERSE(32, 4) | BIT_REVERSE(32, 5) | BIT_REVERSE(32, 15));
    SCCR &= 0xFFFFFF00; // Mask off bits 24 - 31

    retval = wrk_sendOneshot( TAP_WriteDword(0x2FC280, SCCR) );
    if (retval != RET_OK) return retval;

    // mf   = PLPRCR >> 20;
    // divf = PLPRCR & 0x1F;
    PLPRCR &= 0x00FFFFE0;
    // core_castText("MF    : %08X", mf);
    // core_castText("DIVF  : %08X", divf);

    mf   = 0x0D;
    divf = 0x04;

    PLPRCR |= (mf << 20) | divf;
    retval = wrk_sendOneshot( TAP_WriteDword(0x2FC284, PLPRCR) );
    if (retval != RET_OK) return retval;

    //////////////
    // Full blast
    config.Frequency = 12000000;
    retval = wrk_sendOneshot( TAP_SetInterface(config) );

    return retval;
}

uint32_t flashC39(uint32_t Start, uint32_t Length, void *buffer)
{
    uint32_t  i, retval, formatmask = 0;
    uint32_t  curPart,totPart = 0;
    uint32_t *mdLocKeys, *mdRemKeys;
    uint8_t  *bfr_  = buffer;
    uint32_t  Strt_ = Start;
    clock_t   oldTime;
    uint32_t  tmp;

    // Tell core worker NOT to update the progress bar automatically since we're freestyling
    wrk_ManualProgress();

    // Results are currently ignored
    // csum_Check(buffer, EDC16C39_95, bootPartition);

    retval = PrintFlashSignature();
    if (retval != RET_OK) return retval;

    retval = PPC_UploadMD5(0x3F8000);
    if (retval != RET_OK) return 0xffff;

    core_castText("Comparing md5..");

    for (i = 0; i < 32; i++) {
        mdLocKeys = utl_md5Hash(bfr_, 0x010000);
        mdRemKeys = PPC_ReqRemoteMD5(0x3F8000, Strt_, 0x010000);
        if (!mdRemKeys || !mdLocKeys) {
            core_castText("Failed while hashing");
            return 0xFFFF;
        }
        tmp = utl_compareMD5(mdLocKeys, mdRemKeys) << i;
        if (tmp) totPart++;
        formatmask |= tmp;
        Strt_ += 0x010000;
        bfr_  += 0x010000;
        wrk_castProgPub((float)(i/31.0)*100.0);
    }

    if (!formatmask) {
        runDamnit();
        core_castText("Everything is identical");
        return RET_OK;
    }

    core_castText("MASK: %08x", formatmask);

    // Upload driver
    core_castText("Uploading driver..");
    // It should be BE4 but the words are already swapped due to using DMA
    retval = TAP_FillDataBE2(0x3F8000, sizeof(mpc562test_bin), &mpc562test_bin);
    if (retval != RET_OK) {
        core_castText("Fill failed!");
        return retval;
    }
    
    // Store base
    retval = wrk_sendOneshot( TAP_WriteRegister(4, 4, 0) );
    if (retval != RET_OK) {
        core_castText("Failed to configure base");
        return retval;
    }

    // Demand init
    core_castText("Init flash..");
    retval = EDC_DriverDemand(0);
    if (retval != RET_OK) return retval;

    oldTime = clock();
    curPart = 0;
    wrk_castProgPub(0);

    for (i = 0; i < 32; i++) {
        if (formatmask & (1 << i)) {
            core_castText("Erasing partition: %02u (%08X - %08X)", i + 1, i*0x010000, ((i+1)*0x010000)-1);
            retval = edc_SendAddressAndLen(i*0x010000,0x010000);
            if (retval != RET_OK) return retval;
            retval = EDC_DriverDemand(2);
            if (retval != RET_OK) return retval;
            wrk_castProgPub(( (float)++curPart/(float)totPart)*100.0);
        }
    }

    oldTime = (clock() - oldTime) / (CLOCKS_PER_SEC/1000);
    core_castText("Erase took: %u ms", (uint32_t)oldTime);

    oldTime = clock();
    curPart = 0;
    wrk_castProgPub(0);

    // Manipulate buffer for our endian
    wrk_byteSwapBuffer(Length, 2);

    // Store buffer location
    retval = wrk_sendOneshot( TAP_WriteRegister(7, 4, 0x3F8000+2048) );
    if (retval != RET_OK) {
        core_castText("Failed to configure buffer location");
        return retval;
    }

    wrk_ConfigureTimeout(61);

    for (i = 0; i < 32; i++) {
        if (formatmask & (1 << i)) {
            core_castText("Writing partition: %02u (%08X - %08X)", i + 1, i*0x010000, ((i+1)*0x010000)-1);
            retval = edc_SendAddressAndLen(i*0x010000,0x400);
            if (retval != RET_OK){
                core_castText("AddrLen fail");
                return retval;
            }
            // core_castText("Start op");
            retval = wrk_sendOneshot_NoWait(
                TAP_AssistFlash( i*0x010000, 0x010000,
                    0x3F8000,
                    0x3F8000+2048, 0x400 ) );
                    
            if (retval != RET_OK) {
                core_castText("Failed to send AssistFlash command");
                return retval;
            }
            while (!wrk_pollFlashDone() && busyOK() == RET_OK)   ;
            retval = busyOK();
            if (retval != RET_OK) return retval;
            wrk_castProgPub(( (float)++curPart/(float)totPart)*100.0);
            // waitms(500);
        }
    }

    wrk_castProgPub(100);
    oldTime = (clock() - oldTime) / (CLOCKS_PER_SEC/1000);
    core_castText("Flash took: %u ms", (uint32_t)oldTime);

    runDamnit();

    return busyOK();
}

static uint32_t edc_initSPI()
{
    uint32_t  retval;
    uint16_t *ptr;
    uint16_t  PQSPAR;
    uint32_t  i;

    core_castText("Initializing target SPI..");


    /////////////////////////
    // QSMCMMCR: Enable clock
    ptr = wrk_requestData( TAP_ReadWord(0x305000) );
    if (!ptr) return busyOK();
    ptr[2] &= ~BIT_REVERSE(16, 0); // Negate "STOP"
    ptr[2] &= ~BIT_REVERSE(16, 1); // Negate "FRZ"
    ptr[2] |=  BIT_REVERSE(16, 8); // Set "SUPV"
    retval = wrk_sendOneshot( TAP_WriteWord(0x305000, ptr[2]) );
    if (retval != RET_OK) return retval;


    /////////////////////////
    // SPCR1: Negate "SPE" (And everything else, we just want it dead)
    retval = wrk_sendOneshot( TAP_WriteWord(0x30501A, 0x0000) );
    if (retval != RET_OK) return retval;

    wrk_newQueue( TAP_WriteDword(0x305014, 0x00787B7E) ); // PORTQS | PQSPAR | DDRQS
    wrk_queueReq( TAP_WriteDword(0x305018, 0x801C0023) ); // SPCR0. MSTR, Default to 16-bit transfers, 1MHz baud | SPCR1
    wrk_queueReq( TAP_WriteDword(0x30501C, 0x00000000) ); // SPCR2 | SPCR3 / SPSR
    retval = wrk_sendQueue();
    if (retval != RET_OK) return retval;

    // Initialize CMD ram
    // 0x88: "CONT" 1 = Keep CS between transfers. 0 = Release CS
    // 0x28: "DT"   Delay after transfer. 1 = 17 / FSCK, 0 = DTL in SPCR1
    // 0x08: "CS3"  Device to talk to
    // core_castText("Filling command ram..");
    // CMD ram 0x3051C0
    /*
    for (i = 0; i < 32; i++) {
        retval = wrk_sendOneshot( TAP_WriteByte(0x3051C0 + i, 0x21) );
        if (retval != RET_OK) return retval;
    }*/

    return RET_OK;
}

static uint32_t EDC_eepReadStatus(uint16_t *regOut)
{
    uint32_t  retval;
    uint16_t *ptr;

    wrk_newQueue( TAP_WriteByte (0x3051C0, 0x0068) ); // Release cs, 16-bit
    wrk_queueReq( TAP_WriteWord (0x305180, 0x0500) ); // Command: "RDSR"
    wrk_queueReq( TAP_WriteDword(0x30501C, 0x0000) ); // Clear counter, stop at 0
    wrk_queueReq( TAP_WriteWord (0x30501A, 0x8023) ); // Send it!
    retval = wrk_sendQueue();
    if (retval != RET_OK) return retval;

    do{ ptr = wrk_requestData( TAP_ReadWord(0x30501E) );
        if (!ptr) return busyOK();
    } while (!(ptr[2]&BIT_REVERSE(16,8)));

    ptr = wrk_requestData( TAP_ReadWord(0x305140) );
    if (!ptr) return busyOK();

    *regOut = ptr[2]&0xFF;
    return RET_OK;
}

static uint32_t EDC_eepWriteStatus(uint8_t ds)
{
    uint32_t  retval;
    uint16_t *ptr;

    wrk_newQueue( TAP_WriteByte (0x3051C0, 0x0068) ); // Release cs, 16-bit
    wrk_queueReq( TAP_WriteWord (0x305180, 1<<8|ds)); // Command: "WRSR" | new data
    wrk_queueReq( TAP_WriteDword(0x30501C, 0x0000) ); // Clear counter, stop at 0
    wrk_queueReq( TAP_WriteWord (0x30501A, 0x8023) ); // Send it!
    retval = wrk_sendQueue();
    if (retval != RET_OK) return retval;

    do{ ptr = wrk_requestData( TAP_ReadWord(0x30501E) );
        if (!ptr) return busyOK();
    } while (!(ptr[2]&BIT_REVERSE(16,8)));

    return RET_OK;
}

static uint32_t EDC_eepWriteEn()
{
    uint32_t  retval;
    uint16_t *ptr;

    wrk_newQueue( TAP_WriteByte (0x3051C0, 0x0028) ); // Release cs, 8-bit
    wrk_queueReq( TAP_WriteWord (0x305180, 0x0006) ); // Command: "WREN"
    wrk_queueReq( TAP_WriteDword(0x30501C, 0x0000) ); // Clear counter, stop at 0
    wrk_queueReq( TAP_WriteWord (0x30501A, 0x8023) ); // Send it!
    retval = wrk_sendQueue();
    if (retval != RET_OK) return retval;

    do{ ptr = wrk_requestData( TAP_ReadWord(0x30501E) );
        if (!ptr) return busyOK();
    } while (!(ptr[2]&BIT_REVERSE(16,8)));

    return RET_OK;
}

/*
static uint32_t EDC_readEepByte(uint16_t Address, uint16_t *Data)
{
    uint32_t  retval;
    uint16_t *ptr;

    // Set up command queue
    wrk_newQueue( TAP_WriteDword(0x3051C0, 0x88888828 ) );

    // Data to be sent
    wrk_queueReq( TAP_WriteDword(0x305180, 0x03 << 16 | Address >> 8) );
    wrk_queueReq( TAP_WriteDword(0x305184, Address<<16) );

    wrk_queueReq( TAP_WriteDword(0x30501C, 3<<24 ) ); // Ending pointer of the queue | Reset SPCR3 / SPSR
    wrk_queueReq( TAP_WriteWord (0x30501A, 0x8023) ); // Send it!
    retval = wrk_sendQueue();
    if (retval != RET_OK) return retval;

    do{ ptr = wrk_requestData( TAP_ReadWord(0x30501E) );
        if (!ptr) return busyOK();
    } while (!(ptr[2]&BIT_REVERSE(16,8)));

    ptr = wrk_requestData( TAP_ReadWord(0x305146) );
    if (!ptr) return busyOK();
    *Data = ptr[2];

    return RET_OK;
}*/

uint32_t readeepC39(uint32_t Start, uint32_t Length)
{
    uint32_t retval;
    uint16_t data, *ptr;

    retval = edc_initSPI();
    if (retval != RET_OK) return retval;

    // Upload driver
    core_castText("Uploading driver..");
    // It should be BE4 but the words are already swapped due to using DMA
    retval = TAP_FillDataBE2(0x3F8000, sizeof(c39eeprom_bin), &c39eeprom_bin);
    if (retval != RET_OK) return retval;

    core_castText("Reading..");
    // r5: Where is data stored in sram
    // r6: Number of Bytes
    retval = edc_SendAddressAndLen(0x3F8000+4096, 4096);
    if (retval != RET_OK) return retval;

    // r3: 0 Read, 1 Write
    retval = EDC_DriverDemand(0);
    if (retval != RET_OK) return retval;

    // Used while debugging the driver
    // retval = PPC_PrintGPRSum();
    // return retval;

    retval = dumpGenericBE2(0x3F8000+4096, 4096);
    if (retval != RET_OK) return retval;

    return retval;
}

uint32_t writeeepC39(uint32_t Start, uint32_t Length, void *buffer)
{
    uint32_t retval;
    uint16_t data, *ptr;

    retval = edc_initSPI();
    if (retval != RET_OK) return retval;

    // Show current settings
    retval = EDC_eepReadStatus(&data);
    if (retval != RET_OK) return retval;
    core_castText("EEPROM status: %02X", data);

    // Enable write
    retval =  EDC_eepWriteEn();
    if (retval != RET_OK) return retval;

    // Negate BP0, BP1, if set.
    // My firmwares did not lock the eeprom so this is just here as a precation
    retval = EDC_eepWriteStatus(data & ~(0x0C));
    if (retval != RET_OK) return retval;


    // Upload driver
    core_castText("Uploading driver and data..");
    // It should be BE4 but the words are already swapped due to using DMA
    retval = TAP_FillDataBE2(0x3F8000, sizeof(c39eeprom_bin), &c39eeprom_bin);
    if (retval != RET_OK) return retval;

    retval = TAP_FillDataBE2(0x3F8000+4096, 4096, buffer);
    if (retval != RET_OK) return retval;


    core_castText("Writing..");
    // r5: Where is data stored in sram
    // r6: Number of Bytes
    retval = edc_SendAddressAndLen(0x3F8000+4096, 4096);
    if (retval != RET_OK) return retval;

    // r3: 0 Read, 1 Write
    retval = EDC_DriverDemand(1);
    if (retval != RET_OK) return retval;
    
    // Used while debugging the driver
    // retval = PPC_PrintGPRSum();
    // return retval;

    return RET_OK;
}

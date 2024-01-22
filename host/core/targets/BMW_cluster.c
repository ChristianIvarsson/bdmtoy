#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "../core.h"
#include "../core_worker.h"
#include "../core_requests.h"
#include "../core_requests_HCS12.h"

// I'll let this thing speak for itself. This file is a dump
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wint-conversion"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunreachable-code"

#define bs4(a) \
    (((a)&0xffff)<<16 | (((a)>>16)&0xffff))

#define bs2(a) \
    (((a)&0xff) << 24 | (((a)>>8)&0xff) <<16 | (((a)>>16)&0xff) << 8 | (( (a)>>24 )&0xff)  )


// This code has more or less turned into a playground.. Do NOT use!

uint32_t dumpBMWcluster(uint32_t Start, uint32_t Length)
{
    uint32_t fakeAddress;
    uint32_t status = 0;
    uint8_t  page;
    // uint16_t regX;
    // uint16_t *ptr;
/*  
    status = wrk_sendOneshot( HCS12_WritePC(0x1234) );
    if (status != RET_OK)
    {
        core_castText("Fail: %X", status);
        return status;
    }

    ptr = wrk_requestData( HCS12_ReadPC() );
    if (!ptr)
        return busyOK();

    regX = ptr[2];
    core_castText("RegX: %x", regX);
*/
    core_castText("Reading 8 pages..");

    wrk_ResetFault();

    for (page = 0; page < 8; page++)
    {
        fakeAddress = 0x4000 * (page&0xF);
        core_castText("Page 0x%02X (0x%05X - 0x%05X)", page, fakeAddress, fakeAddress + 0x3FFF);
        
        // Activate page of interest
        status = wrk_sendOneshot( TAP_WriteByte(0xFF, page) );
        if (status != RET_OK)
        {
            core_castText("Fail: %X", status);
            return status;
        }
        // Send dump request
        status = wrk_sendOneshot_NoWait(TAP_Dump(0x8000, 0x4000));
        if (status != RET_OK)
            return status;
    
        while (wrk_dumpDone() == RET_BUSY)
        {
            status = busyOK();
            if (status != RET_OK)
                return status;
        }
    }

    wrk_byteSwapBuffer(Length, 2);

    return status;
}

uint32_t initBMWcluster()
{
    TAP_Config_host_t config;
    uint32_t retval;
    uint32_t Retries = 3;
    uint16_t *ptr;

    config.Type           = TAP_IO_BDMS;
    config.Frequency      = 4000000/2;
    config.cfgmask.Endian = TAP_BIGENDIAN;
    
Retry:
    wrk_ResetFault();
    retval = wrk_sendOneshot(TAP_SetInterface(config));
    if (retval != RET_OK)
    {
        if (Retries--)
            goto Retry;
        return retval;
    }
    
    retval = wrk_sendOneshot(TAP_TargetReady());
    if (retval != RET_OK)
    {
        if (Retries--)
            goto Retry;
        
    }

    // Faster for ef sake...
    // This is max what the adapter can achieve more or less. 2 host cycles to spare!
    config.Frequency = 8000000;

    ptr = wrk_requestData( TAP_ReadByte(0x3D) );
    if (!ptr)
        return 0xFFFF;
    core_castText ("CLKSEL: %02x",ptr[2]); // BCSP = bit 6

/*
    if ( wrk_sendOneshot( TAP_WriteByte(0x0038, 1) ) != RET_OK )
    {
        core_castText("Failed to configure multiplier");
        return 0xFFFF;
    }

    if ( wrk_sendOneshot( TAP_WriteByte(0x003D, 0x40) ) != RET_OK )
    {
        core_castText("Failed to enable PLL");
        return 0xFFFF;
    }

    ptr = wrk_requestData( HCS12_ReadBDMAddress(0x01) );
    if (!ptr)
        return 0xFFFF;

    if ( wrk_sendOneshot( HCS12_WriteBDMAddress(0x01, ptr[2] | 4) ) != RET_OK )
    {
        core_castText("Failed configure BDM as driven by PLL");
        return 0xFFFF;
    }

    if ( wrk_sendOneshot(TAP_SetInterface(config) ) != RET_OK )
    {
        core_castText("Failed to set adapter speed");
        return 0xFFFF;
    }

*/

    if ( wrk_sendOneshot( TAP_WriteByte(0x0016, 0x08) ) != RET_OK )
    {
        core_castText("Failed disable COP");
        return 0xFFFF;
    }

    if ( wrk_sendOneshot( HCS12_WriteSP(0x2FF0) ) != RET_OK )
    {
        core_castText("Failed to set SP");
        return 0xFFFF;
    }

    return retval;
}

uint32_t showABCD()
{
    uint32_t A,B,C,D,tA,tB,tC,tD,tmp/*,flash*/;
    uint16_t *ptr;
    uint32_t *cheat = (uint32_t *) 0x2100;

    ptr = wrk_requestData( TAP_ReadDword(cheat++) ); // Since it's a "pointer", it'll increment by four instead of one. Just ignore the stupid warning..
    if (!ptr)
        return 0xFFFF;
    A = bs4(*(uint32_t *) &ptr[2]);

    ptr = wrk_requestData( TAP_ReadDword(cheat++) );
    if (!ptr)
        return 0xFFFF;
    B = bs4(*(uint32_t *) &ptr[2]);

    ptr = wrk_requestData( TAP_ReadDword(cheat++) );
    if (!ptr)
        return 0xFFFF;
    C = bs4(*(uint32_t *) &ptr[2]);

    ptr = wrk_requestData( TAP_ReadDword(cheat++) );
    if (!ptr)
        return 0xFFFF;
    D = bs4(*(uint32_t *) &ptr[2]);

    ptr = wrk_requestData( TAP_ReadDword(cheat++) );
    if (!ptr)
        return 0xFFFF;
    tA = bs4(*(uint32_t *) &ptr[2]);

    ptr = wrk_requestData( TAP_ReadDword(cheat++) );
    if (!ptr)
        return 0xFFFF;
    tB = bs4(*(uint32_t *) &ptr[2]);

    ptr = wrk_requestData( TAP_ReadDword(cheat++) );
    if (!ptr)
        return 0xFFFF;
    tC = bs4(*(uint32_t *) &ptr[2]);

    ptr = wrk_requestData( TAP_ReadDword(cheat++) );
    if (!ptr)
        return 0xFFFF;
    tD = bs4(*(uint32_t *) &ptr[2]);


    ptr = wrk_requestData( TAP_ReadDword(cheat++) );
    if (!ptr)
        return 0xFFFF;
    tmp = bs4(*(uint32_t *) &ptr[2]);

/*
    ptr = wrk_requestData( TAP_ReadDword(0xc000) );
    if (!ptr)
        return 0xFFFF;
    flash = bs4(*(uint32_t *) &ptr[2]);
*/
    // core_castText(" A: 0x%08X  B: 0x%08X  C: 0x%08X  D: 0x%08X",A,B,C,D);
    core_castText("tA: 0x%08X tB: 0x%08X tC: 0x%08X tD: 0x%08X",bs2(tA),bs2(tB),bs2(tC),bs2(tD));
    // core_castText("TMP: %08X", tmp);
    // core_castText("flash: %08X", flash);
    return RET_OK;
}









uint32_t checksum_data(uint32_t start, uint32_t length, void *data)
{
    // uint16_t *ptr;
    uint8_t *dataptr = (uint8_t *) data;
    uint32_t checksum = 0;
    uint32_t i;

    for (i = 0; i < (length - 0xA8); i++)
    {
        checksum += dataptr[0xa8 + i];
    }

    core_castText("Local checksum: %08x", checksum);
/*
    checksum = 0;
    start += 0xa8;
    length -= 0xa8;

    while (length-=2)
    {
        ptr = wrk_requestData( TAP_ReadWord(start) );
        if (!ptr)
            return 0xFFFF;
        checksum += ptr[2]&0xFF;
        checksum += (ptr[2]>>8)&0xFF;

        start += 2;

        // core_castText ("COPCTL: %02x",ptr[2]);
    }

    core_castText("Remote checksum: %08x", checksum);*/
    return 0;

}



uint32_t fillbuffer_a()
{
    uint16_t address = 0x3000;
    uint32_t i;

    uint16_t data[0x1000/2];

    for (i = 0; i < 0x1000/2; i++)
        data[i]=i;

    // data[0xb40/2] = 0xFFFF;
    // data[0xb42/2] = 0xFFFF;
    return TAP_FillDataBE2(0x3000, 0x1000, data);
}

uint32_t fillbuffer_b()
{
    uint16_t address = 0x3000;
    uint32_t i;

    uint16_t data[0x1000/2];

    for (i = 0; i < 0x1000/2; i++)
        data[i]=0x2233;


    return TAP_FillDataBE2(0x3000, 0x1000, data);
}


// Guess why I had to implement this....
// It's an EXACT copy of Motorola's procedure. Excuse the weird order
uint32_t secureErase()
{
    TAP_Config_host_t config;
    uint32_t retval;
    uint16_t *ptr;
    uint8_t  tmp;

    config.Type           = TAP_IO_BDMS;
    config.Frequency      = 4194000/2;
    config.cfgmask.Endian = TAP_BIGENDIAN;

    core_castText("Attempting bulk erase..");

    wrk_ResetFault();
    retval = wrk_sendOneshot(TAP_SetInterface(config));
    if (retval != RET_OK)
    {
        core_castText("Failed set interface");
        return 0xFFFF;
    }

    // Perform a reset-stop
    retval = wrk_sendOneshot(TAP_TargetReady());
    if (retval != RET_OK)
    {
        core_castText("Failed to ready target");
        return 0xFFFF;
    }

    waitms(2000);

    // Set eeprom speed
    if ( wrk_sendOneshot( TAP_WriteByte(0x0110, 10) ) != RET_OK )
    {
        core_castText("Failed to set eeprom speed");
        return 0xFFFF;
    }

    // Disable eeprom protection
    if ( wrk_sendOneshot( TAP_WriteByte(0x0114, 0xFF) ) != RET_OK )
    {
        core_castText("Failed to clear EPROT");
        return 0xFFFF;
    }

    // Setup ESTAT
    if ( wrk_sendOneshot( TAP_WriteByte(0x0115, 0x30) ) != RET_OK )
    {
        core_castText("Failed to clear ESTAT");
        return 0xFFFF;
    }

    if ( wrk_sendOneshot( TAP_WriteWord(0x011A, 0) ) != RET_OK )
    {
        core_castText("Failed to set eeprom data");
        return 0xFFFF;
    }
    if ( wrk_sendOneshot( TAP_WriteWord(0x0118, 0) ) != RET_OK )
    {
        core_castText("Failed to set eeprom address");
        return 0xFFFF;
    }

    // eeprom command
    if ( wrk_sendOneshot( TAP_WriteByte(0x0116, 0x41) ) != RET_OK )
    {
        core_castText("Failed to send eeprom command");
        return 0xFFFF;
    }

    if ( wrk_sendOneshot( TAP_WriteByte(0x0115, 0x80) ) != RET_OK )
    {
        core_castText("Failed to clear ESTAT CBEIF");
        return 0xFFFF;
    }

    // Set flash speed
    if ( wrk_sendOneshot( TAP_WriteByte(0x0100, 10) ) != RET_OK )
    {
        core_castText("Failed to set flash speed");
        return 0xFFFF;
    }

    if ( wrk_sendOneshot( TAP_WriteByte(0x0103, 0) ) != RET_OK )
    {
        core_castText("Failed to set flash block 0");
        return 0xFFFF;
    }

    if ( wrk_sendOneshot( TAP_WriteByte(0x0102, 0x10) ) != RET_OK )
    {
        core_castText("Failed to set wrall");
        return 0xFFFF;
    }

    // Disable flash protection
    if ( wrk_sendOneshot( TAP_WriteByte(0x0104, 0xFF) ) != RET_OK )
    {
        core_castText("Failed to clear EPROT");
        return 0xFFFF;
    }

    if ( wrk_sendOneshot( TAP_WriteByte(0x0105, 0x30) ) != RET_OK )
    {
        core_castText("Failed to clear fstat");
        return 0xFFFF;
    }

    if ( wrk_sendOneshot( TAP_WriteWord(0x010A, 0) ) != RET_OK )
    {
        core_castText("Failed to set flash data");
        return 0xFFFF;
    }
    if ( wrk_sendOneshot( TAP_WriteWord(0x0108, 0) ) != RET_OK )
    {
        core_castText("Failed to set flash address");
        return 0xFFFF;
    }

    if ( wrk_sendOneshot( TAP_WriteByte(0x0106, 0x41) ) != RET_OK )
    {
        core_castText("Failed to send flash erase command");
        return 0xFFFF;
    }

    if ( wrk_sendOneshot( TAP_WriteByte(0x0105, 0x80) ) != RET_OK )
    {
        core_castText("Failed to clear fstat cbeif");
        return 0xFFFF;
    }

eepwait:
    ptr = wrk_requestData( TAP_ReadByte(0x0115) );
    if (!ptr) return 0xFFFF;
    tmp = ptr[2];
    if ( !(tmp & 0x40) ) goto eepwait;

flashwait:
    ptr = wrk_requestData( TAP_ReadByte(0x0105) );
    if (!ptr) return 0xFFFF;
    tmp = ptr[2];
    if ( !(tmp & 0x40) ) goto flashwait;
    

    wrk_ResetFault();
    retval = wrk_sendOneshot(TAP_SetInterface(config));
    if (retval != RET_OK)
    {
        core_castText("Failed set interface");
        return 0xFFFF;
    }

    // Perform a reset-stop
    retval = wrk_sendOneshot(TAP_TargetReady());
    if (retval != RET_OK)
    {
        core_castText("Failed to ready target");
        return 0xFFFF;
    }

    waitms(2000);


    // Set flash speed
    if ( wrk_sendOneshot( TAP_WriteByte(0x0100, 10) ) != RET_OK )
    {
        core_castText("Failed to set flash speed");
        return 0xFFFF;
    }

    if ( wrk_sendOneshot( TAP_WriteByte(0x0103, 0) ) != RET_OK )
    {
        core_castText("Failed to set flash block 0");
        return 0xFFFF;
    }

    if ( wrk_sendOneshot( TAP_WriteByte(0x0104, 0xff) ) != RET_OK )
    {
        core_castText("Failed to disable flash protection");
        return 0xFFFF;
    }

    if ( wrk_sendOneshot( TAP_WriteWord(0x010A, 0xFFFE) ) != RET_OK ) // FFBE
    {
        core_castText("Failed to set flash data");
        return 0xFFFF;
    }
    if ( wrk_sendOneshot( TAP_WriteWord(0x0108, 0x8000 | (0xFF0E>>1)) ) != RET_OK )
    {
        core_castText("Failed to set flash address");
        return 0xFFFF;
    }

    if ( wrk_sendOneshot( TAP_WriteByte(0x0106, 0x20) ) != RET_OK )
    {
        core_castText("Failed to send flash command");
        return 0xFFFF;
    }

    if ( wrk_sendOneshot( TAP_WriteByte(0x0105, 0x80) ) != RET_OK )
    {
        core_castText("Failed to send flash command");
        return 0xFFFF;
    }

writewait:
    ptr = wrk_requestData( TAP_ReadByte(0x0105) );
    if (!ptr) return 0xFFFF;
    tmp = ptr[2];
    if ( !(tmp & 0x40) ) goto writewait;

    wrk_ResetFault();
    retval = wrk_sendOneshot(TAP_SetInterface(config));
    if (retval != RET_OK)
    {
        core_castText("Failed set interface");
        return 0xFFFF;
    }

    // Perform a reset-stop
    retval = wrk_sendOneshot(TAP_TargetReady());
    if (retval != RET_OK)
    {
        core_castText("Failed to ready target");
        return 0xFFFF;
    }

    waitms(500);

    return 0;    
}

#define SIUFLASHSP   41

uint32_t initializeFlashSIU_()
{
    if ( wrk_sendOneshot( TAP_WriteByte(0x0100,   SIUFLASHSP) ) != RET_OK )
    {
        core_castText("Failed to set flash speed");
        return 0xFFFF;
    }
    if ( wrk_sendOneshot( TAP_WriteByte(0x0105, 0x30) ) != RET_OK )
    {
        core_castText("Failed to set flash speed");
        return 0xFFFF;
    }
    if ( wrk_sendOneshot( TAP_WriteByte(0x0103, 0x00) ) != RET_OK )
    {
        core_castText("Failed to set flash speed");
        return 0xFFFF;
    }
    return RET_OK;

}

uint32_t initializeEepSIU_()
{
    if ( wrk_sendOneshot( TAP_WriteByte(0x0110,   SIUFLASHSP) ) != RET_OK )
    {
        core_castText("Failed to set eeprom speed");
        return 0xFFFF;
    }
    if ( wrk_sendOneshot( TAP_WriteByte(0x0115, 0x30) ) != RET_OK )
    {
        core_castText("Failed to set eeprom speed");
        return 0xFFFF;
    }
    if ( wrk_sendOneshot( TAP_WriteByte(0x0113, 0x00) ) != RET_OK )
    {
        core_castText("Failed to set eeprom speed");
        return 0xFFFF;
    }
    return RET_OK;

}












uint32_t playwithEEP(uint32_t Address, uint32_t Length, void *buffer)
{
    uint16_t *ptr;
    uint16_t sp;
    // uint16_t checksumptr;
    // uint32_t checksum;


    uint16_t eepaddr = 0x1000;

    if ( wrk_sendOneshot( TAP_WriteByte(0x0013,   0) ) != RET_OK )
    {
        core_castText("Failed to disable flash");
        return 0xFFFF;
    }

    if ( wrk_sendOneshot( TAP_WriteByte(0x0012,   0x11) ) != RET_OK )
    {
        core_castText("Failed to enable EEP");
        return 0xFFFF;
    }

    initializeEepSIU_();
    ptr = wrk_requestData( TAP_ReadDword(eepaddr) );
    if (!ptr) return 0xFFFF;
    core_castText("Data: %04X", *(uint32_t *) &ptr[2]);
    

    ptr = wrk_requestData( HCS12_ReadSP() );
    if (!ptr) return 0xFFFF;

    sp = ptr[2];


    core_castText("sp: %04x", sp);

    if (fillbuffer_a() != RET_OK)
        return 0xFFFF;

    // Length
    if ( wrk_sendOneshot( TAP_WriteWord(sp + 8, 12) ) != RET_OK )
    {
        core_castText("Failed to store length or mask");
        return 0xFFFF;
    }

    // Eeprom pointer
    if ( wrk_sendOneshot( TAP_WriteWord(sp + 6, eepaddr) ) != RET_OK )
    {
        core_castText("Failed to store flash pointer");
        return 0xFFFF;
    }

    // Buffer pointer
    if ( wrk_sendOneshot( TAP_WriteWord(sp + 4, 0x3000) ) != RET_OK )
    {
        core_castText("Failed to store buffer pointer");
        return 0xFFFF;
    }

    // Operation
    if ( wrk_sendOneshot( TAP_WriteWord(sp + 2, 0x0001) ) != RET_OK )
    {
        core_castText("Failed to store mode of operation");
        return 0xFFFF;
    }

    HCS12_PrintRegSummary();
    HCS12_PrintStackContents(5);
    
    // ..
    core_castText("Uploading driver..");
    if ( TAP_FillDataBE2(Address, Length, buffer) != RET_OK )
    {
        core_castText("Failed to upload data");
        return 0xFFFF;
    }


    core_castText("Running code..");
    if ( wrk_sendOneshot( HCS12_WritePC(Address) ) != RET_OK )
    {
        core_castText("Failed to set PC");
        return 0xFFFF;
    }

    if ( wrk_sendOneshot( TAP_TargetStart() ) != RET_OK )
    {
        core_castText("Failed to start target");
        return 0xFFFF;
    }

    if ( HCS12_WaitBDM() != RET_OK )
    {
        core_castText("Failed while waiting for target to stop");
        return 0xFFFF;
    }

    // sleep(1);

    HCS12_PrintStackContents(5);
    HCS12_PrintRegSummary();

    ptr = wrk_requestData( TAP_ReadDword(eepaddr) );
    if (!ptr) return 0xFFFF;
    core_castText("Data: %04X", *(uint32_t *) &ptr[2]);

    ptr = wrk_requestData( TAP_ReadDword(eepaddr+4) );
    if (!ptr) return 0xFFFF;
    core_castText("Data: %04X", *(uint32_t *) &ptr[2]);

    ptr = wrk_requestData( TAP_ReadDword(eepaddr+8) );
    if (!ptr) return 0xFFFF;
    core_castText("Data: %04X", *(uint32_t *) &ptr[2]);

    return RET_OK;
}

uint32_t eraseFlash_BMWcluser(uint32_t Address, uint32_t Length, void *buffer)
{
    uint16_t *ptr;
    uint16_t sp;
    // uint16_t checksumptr;
    // uint32_t checksum;


    // secureErase();
    initializeFlashSIU_();
    initializeEepSIU_();





    return playwithEEP(Address, Length, buffer);






    // if ( wrk_requestData( HCS12_WriteBDMAddress(1, 0xC0) ) != RET_OK )
    // return 0xFFFF;

    ptr = wrk_requestData( HCS12_ReadBDMAddress(1) );
    if (!ptr) return 0xFFFF;
    core_castText("bdm1: %02X", ptr[2]);

    // return 0;

    // SP: 0x2FF0

    ptr = wrk_requestData( HCS12_ReadSP() );
    if (!ptr) return 0xFFFF;

    sp = ptr[2];


    core_castText("sp: %04x", sp);

    // return 0;

    if (fillbuffer_a() != RET_OK)
        return 0xFFFF;

    ptr = wrk_requestData( TAP_ReadDword(0x3ff0) );
    if (!ptr) return 0xFFFF;
    core_castText("rambuffer: %04X", *(uint32_t *) &ptr[2]);


    ptr = wrk_requestData( TAP_ReadWord(0x1000) );
    if (!ptr) return 0xFFFF;
    core_castText("eeprom: %04X", *(uint32_t *) &ptr[2]);


    // Page
    if ( wrk_sendOneshot( TAP_WriteByte(sp + 10, 0x38) ) != RET_OK )
    {
        core_castText("Failed to store length or mask");
        return 0xFFFF;
    }


    // Length
    if ( wrk_sendOneshot( TAP_WriteWord(sp + 8, 0x1000) ) != RET_OK )
    {
        core_castText("Failed to store length or mask");
        return 0xFFFF;
    }

    // Flash pointer
    if ( wrk_sendOneshot( TAP_WriteWord(sp + 6, 0x8000) ) != RET_OK )
    {
        core_castText("Failed to store flash pointer");
        return 0xFFFF;
    }

    // Buffer pointer
    if ( wrk_sendOneshot( TAP_WriteWord(sp + 4, 0x3000) ) != RET_OK )
    {
        core_castText("Failed to store buffer pointer");
        return 0xFFFF;
    }

    // Operation
    if ( wrk_sendOneshot( TAP_WriteWord(sp + 2, 0x0001) ) != RET_OK )
    {
        core_castText("Failed to store mode of operation");
        return 0xFFFF;
    }

    HCS12_PrintRegSummary();
    HCS12_PrintStackContents(5);
    
    // ..
    core_castText("Uploading driver..");
    if ( TAP_FillDataBE2(Address, Length, buffer) != RET_OK )
    {
        core_castText("Failed to upload data");
        return 0xFFFF;
    }


    core_castText("Running code..");
    if ( wrk_sendOneshot( HCS12_WritePC(Address) ) != RET_OK )
    {
        core_castText("Failed to set PC");
        return 0xFFFF;
    }

    if ( wrk_sendOneshot( TAP_TargetStart() ) != RET_OK )
    {
        core_castText("Failed to start target");
        return 0xFFFF;
    }

    if ( HCS12_WaitBDM() != RET_OK )
    {
        core_castText("Failed while waiting for target to stop");
        return 0xFFFF;
    }



    HCS12_PrintStackContents(5);
    HCS12_PrintRegSummary();


    wrk_sendOneshot( TAP_WriteByte(0x30, 0x30) );

    ptr = wrk_requestData( TAP_ReadDword(0x8000) );
    if (!ptr) return 0xFFFF;
    core_castText("Data: %04X", *(uint32_t *) &ptr[2]);

    ptr = wrk_requestData( TAP_ReadDword(0x8004) );
    if (!ptr) return 0xFFFF;
    core_castText("Data: %04X", *(uint32_t *) &ptr[2]);



    wrk_sendOneshot( TAP_WriteByte(0x30, 0x31) );

    ptr = wrk_requestData( TAP_ReadDword(0x8000) );
    if (!ptr) return 0xFFFF;
    core_castText("Data: %04X", *(uint32_t *) &ptr[2]);






    wrk_sendOneshot( TAP_WriteByte(0x30, 0x3F) );

    ptr = wrk_requestData( TAP_ReadWord(0xBF0E) );
    if (!ptr) return 0xFFFF;
    core_castText("Data: %04X", ptr[2]);









    wrk_sendOneshot( TAP_WriteByte(0x30, 0x38) );

    ptr = wrk_requestData( TAP_ReadDword(0xAB40) );
    if (!ptr) return 0xFFFF;
    core_castText("b40: %04X", *(uint32_t *) &ptr[2]);



/*
    // Read Y to determine where the checksum is located
    ptr = wrk_requestData( HCS12_ReadY() );
    if (!ptr) return 0xFFFF;

    checksumptr = ptr[2];

    ptr = wrk_requestData( TAP_ReadDword(checksumptr) );
    if (!ptr) return 0xFFFF;*/

    // checksum = bs4(*(uint32_t *) &ptr[2]);

    // core_castText("Checksum: %08X", checksum );

    return RET_OK;

}



uint32_t runSRAM_BMWcluster(uint32_t Address, uint32_t Length, void *buffer)
{

    return eraseFlash_BMWcluser(Address, Length, buffer);

    core_castText("Before operation:");
    HCS12_PrintRegSummary();

    

    // ..
    core_castText("Uploading data..");
    if ( TAP_FillDataBE2(Address, Length, buffer) != RET_OK )
    {
        core_castText("Failed to upload data");
        return 0xFFFF;
    }
    // checksum_data(Address, Length, buffer);
    showABCD();
/*
    ptr = wrk_requestData( TAP_ReadByte(0x0016) );
    if (!ptr)
        return 0xFFFF;
    core_castText ("COPCTL: %02x",ptr[2]);
    // A = bs4(*(uint32_t *) &ptr[2]);
*/
    if ( wrk_sendOneshot( TAP_WriteByte(0x0016, 0x08) ) != RET_OK )
    {
        core_castText("Failed disable COP");
        return 0xFFFF;
    }
/*    
    ptr = wrk_requestData( TAP_ReadByte(0x0016) );
    if (!ptr)
        return 0xFFFF;
    core_castText ("COPCTL: %02x",ptr[2]);
*/
    core_castText("Running code..");
    if ( wrk_sendOneshot( HCS12_WritePC(Address) ) != RET_OK )
    {
        core_castText("Failed to set PC");
        return 0xFFFF;
    }
/*
    ptr = wrk_requestData( TAP_ReadWord(Address) );
    if (!ptr)
        return busyOK();

    core_castText("bdm 0x01: %x", ptr[2]);
*/


    // Run code..
    if ( wrk_sendOneshot( TAP_TargetStart() ) != RET_OK )
    {
        core_castText("Failed to start target");
        return 0xFFFF;
    }

    if ( HCS12_WaitBDM() != RET_OK )
    {
        core_castText("Failed while waiting for target to stop");
        return 0xFFFF;
    }

    // checksum_data(Address, Length, buffer);

    showABCD();

    core_castText("After operation:");
    HCS12_PrintRegSummary();
    // TAP_FillDataBE2()
}


// Ah.. the silence was nice. Let's restore the whine
#pragma GCC diagnostic pop

#ifdef __cplusplus 
}
#endif

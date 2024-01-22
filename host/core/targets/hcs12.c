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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-value"
#pragma GCC diagnostic ignored "-Wint-conversion"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunreachable-code"

#define SIDMULTI      1
#define SIDFLASHSP   41
// 8 388000 / 200000 = 41,94
// 8 388000 / 42 = 199714
// fclkdiv: 41 (+ 1)

static const uint8_t sidFlashDrv[] = {
    0xE6, 0x8A, 0xCE, 0x00, 0x30, 0x6B, 0x00, 0x86, 0x3F, 0x18, 0x16, 0x44,
    0x44, 0x84, 0x07, 0xCE, 0x01, 0x00, 0x6A, 0x03, 0x86, 0xFF, 0x6A, 0x04,
    0xEC, 0x88, 0x6C, 0x82, 0xEE, 0x84, 0xED, 0x86, 0x35, 0x34, 0x86, 0x00,
    0x36, 0xEC, 0x00, 0xAC, 0x40, 0x26, 0x0E, 0x19, 0x42, 0x1A, 0x02, 0x63,
    0x80, 0x26, 0xF2, 0x1B, 0x85, 0x18, 0x20, 0x00, 0x6A, 0x1B, 0x81, 0x30,
    0x31, 0x34, 0xCE, 0x01, 0x00, 0x6C, 0x40, 0x86, 0x40, 0x6A, 0x06, 0x86,
    0x80, 0x6A, 0x05, 0xA6, 0x05, 0x84, 0x30, 0x27, 0x04, 0x6A, 0x05, 0x20,
    0xEC, 0x1F, 0x01, 0x05, 0x40, 0xFB, 0xC6, 0x08, 0x37, 0xC6, 0x20, 0x37,
    0xEE, 0x82, 0xEC, 0x00, 0x6C, 0x40, 0xCE, 0x01, 0x00, 0x86, 0x20, 0x6A,
    0x06, 0xA6, 0x05, 0x8A, 0x80, 0x6A, 0x05, 0xA6, 0x05, 0x84, 0x30, 0x27,
    0x04, 0x6A, 0x05, 0x20, 0xE3, 0xEC, 0x82, 0xC3, 0x00, 0x02, 0x6C, 0x82,
    0x19, 0x42, 0x63, 0x80, 0xA6, 0x05, 0x84, 0x80, 0x27, 0x04, 0xA6, 0x80,
    0x26, 0xCE, 0x1F, 0x01, 0x05, 0x40, 0xFB, 0xA6, 0x80, 0x26, 0xC5, 0x1B,
    0x81, 0x63, 0x80, 0x26, 0xBC, 0x33, 0x30, 0x63, 0x82, 0x63, 0x82, 0x18,
    0x26, 0xFF, 0x75, 0xCC, 0x00, 0x01, 0x6C, 0x82, 0x6D, 0x86, 0x00
};

static const uint8_t sidEepDrv[] = {
    0xCE, 0x01, 0x10, 0x86, 0xFF, 0x6A, 0x04, 0xEC, 0x88, 0x6C, 0x82, 0xEE,
    0x84, 0xED, 0x86, 0x35, 0x34, 0x86, 0x02, 0x36, 0xEC, 0x00, 0xAC, 0x40,
    0x26, 0x0E, 0x19, 0x42, 0x1A, 0x02, 0x63, 0x80, 0x26, 0xF2, 0x1B, 0x85,
    0x18, 0x20, 0x00, 0x57, 0x1B, 0x81, 0x30, 0x31, 0x34, 0xCE, 0x01, 0x10,
    0x6C, 0x40, 0x86, 0x40, 0x6A, 0x06, 0x86, 0x80, 0x6A, 0x05, 0xA6, 0x05,
    0x84, 0x30, 0x27, 0x04, 0x6A, 0x05, 0x20, 0xEC, 0x1F, 0x01, 0x15, 0x40,
    0xFB, 0xC6, 0x02, 0x37, 0xEE, 0x81, 0xEC, 0x00, 0x6C, 0x40, 0xCE, 0x01,
    0x10, 0x86, 0x20, 0x6A, 0x06, 0xA6, 0x05, 0x8A, 0x80, 0x6A, 0x05, 0xA6,
    0x05, 0x84, 0x30, 0x27, 0x04, 0x6A, 0x05, 0x20, 0xE3, 0xEC, 0x81, 0xC3,
    0x00, 0x02, 0x6C, 0x81, 0x19, 0x42, 0x63, 0x80, 0x1F, 0x01, 0x15, 0x40,
    0xFB, 0xA6, 0x80, 0x26, 0xCF, 0x33, 0x30, 0xEC, 0x82, 0xC3, 0xFF, 0xFC,
    0x6C, 0x82, 0x18, 0x26, 0xFF, 0x85, 0xCC, 0x00, 0x01, 0x6C, 0x82, 0x6D,
    0x86, 0x00
};

uint32_t initializeFlashSID()
{
    if ( wrk_sendOneshot( TAP_WriteByte(0x0100,   SIDFLASHSP) ) != RET_OK )
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

uint32_t initializeEepSID()
{
    if ( wrk_sendOneshot( TAP_WriteByte(0x0110,   SIDFLASHSP) ) != RET_OK )
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

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Flash / Dump functions
static uint32_t sid_write64(uint32_t address, uint8_t page, uint16_t sp, void *buffer)
{
    uint16_t *writeBuffer = (uint16_t *) buffer;
    uint16_t bufferLen = 0x1000;
    uint16_t bufPtr    = 0x3000;
    uint16_t dataLeft;
    uint16_t *ptr;

oddPage:
    core_castText("Page %02X (0x%05X - 0x%05X)", page, address, address+0x3FFF);
    dataLeft  = 16384;

    // Page
    if ( wrk_sendOneshot( TAP_WriteByte(sp + 10, page++) ) != RET_OK )
    {
        core_castText("Failed to store page");
        return 0xFFFF;
    }

    // Flash pointer
    if ( wrk_sendOneshot( TAP_WriteWord(sp + 6, 0x8000 + (address&0x3FFF) ) ) != RET_OK )
    {
        core_castText("Failed to store flash pointer");
        return 0xFFFF;
    }

    // Buffer pointer
    if ( wrk_sendOneshot( TAP_WriteWord(sp + 4, bufPtr) ) != RET_OK )
    {
        core_castText("Failed to store buffer pointer");
        return 0xFFFF;
    }

    while (dataLeft)
    {
        if ( TAP_FillDataBE2(bufPtr, bufferLen, writeBuffer) != RET_OK )
        {
            core_castText("Failed to upload data");
            return 0xFFFF;
        }  

        if ( wrk_sendOneshot( HCS12_WritePC(0x2000) ) != RET_OK )
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

        ptr = wrk_requestData( HCS12_ReadD() );
        if (!ptr)
        {
            core_castText("Could not read flash status");
            return 0xFFFF;
        }

        if (ptr[2] != 1)
        {
            core_castText("Target flagged %x as fault", ptr[2]);
            return 0xFFFF;
        }

        address     += bufferLen;
        writeBuffer += bufferLen/2;
        dataLeft    -= bufferLen;
        wrk_castProgPub( 100 * ((float)(address) / (float)(0x40000)) );
    }

    if (page&3)
        goto oddPage;

    return RET_OK;
}

uint32_t flashSID95(uint32_t Start, uint32_t Length, void *buffer)
{
    uint16_t *writeBuffer = (uint16_t *) buffer;
    uint8_t  page = 0x40 - (Length / 16384);
    uint16_t *ptr;
    uint16_t sp;

    wrk_ResetFault();

    if ( initializeFlashSID() != RET_OK )
        return 0xFFFF;

    ptr = wrk_requestData( HCS12_ReadSP() );
    if (!ptr)
    {
        core_castText("Could not read current SP");
        return 0xFFFF;
    }
    sp = ptr[2];

    // ..
    core_castText("Uploading driver");
    if ( TAP_FillDataBE2(0x2000, sizeof(sidFlashDrv), &sidFlashDrv) != RET_OK )
    {
        core_castText("Failed to upload data");
        return 0xFFFF;
    }

    // Length
    if ( wrk_sendOneshot( TAP_WriteWord(sp + 8, 0x1000) ) != RET_OK )
    {
        core_castText("Failed to store length");
        return 0xFFFF;
    }

    // Operation
    if ( wrk_sendOneshot( TAP_WriteWord(sp + 2, 0x0001) ) != RET_OK )
    {
        core_castText("Failed to store mode of operation");
        return 0xFFFF;
    }

    core_castText("Writing %u pages..", 0x40 - page);
    // HCS12_PrintRegSummary();
    while (Start < Length)
    {
        if ( sid_write64(Start, page, sp, writeBuffer) != RET_OK )
            return 0xffff;

        writeBuffer += 32768;
        Start       += 65536;
        page += 4;
    }

    return RET_OK;
} 

uint32_t dumpSID95(uint32_t Start, uint32_t Length)
{
    uint8_t  page = 0x40 - (Length / 16384);
    uint32_t status = 0, fakeAddress = 0;

    wrk_ResetFault();

    core_castText("Reading %u pages..", 0x40 - page);

    for ( ; page < 0x40; page++)
    {
        core_castText("Page 0x%02X (0x%05X - 0x%05X)", page, fakeAddress, fakeAddress + 0x3FFF);
        fakeAddress += 0x4000;
        
        // Activate page of interest
        status = wrk_sendOneshot( TAP_WriteByte(0x30, page) );
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

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Eeprom functions
uint32_t writeeepSID95(uint32_t Start, uint32_t Length, void *buffer)
{
    uint16_t *ptr;
    uint16_t sp;
    
    wrk_ResetFault();

    if ( wrk_sendOneshot( TAP_WriteByte(0x0013,   0) ) != RET_OK )
    {
        core_castText("Failed to disable flash");
        return 0xFFFF;
    }

    if ( wrk_sendOneshot( TAP_WriteByte(0x0012,   (Start>>8) | 1) ) != RET_OK )
    {
        core_castText("Failed to enable EEP");
        return 0xFFFF;
    }

    if ( initializeEepSID() != RET_OK )
        return 0xFFFF;

    ptr = wrk_requestData( HCS12_ReadSP() );
    if (!ptr)
    {
        core_castText("Could not read current SP");
        return 0xFFFF;
    }
    sp = ptr[2];

    core_castText("Uploading driver");
    if ( TAP_FillDataBE2(0x2000, sizeof(sidEepDrv), &sidEepDrv) != RET_OK )
    {
        core_castText("Failed to upload data");
        return 0xFFFF;
    }

    // Length
    if ( wrk_sendOneshot( TAP_WriteWord(sp + 8, Length) ) != RET_OK )
    {
        core_castText("Failed to store length");
        return 0xFFFF;
    }

    // Eeprom pointer
    if ( wrk_sendOneshot( TAP_WriteWord(sp + 6, Start) ) != RET_OK )
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
    
    core_castText("Filling buffer");
    if ( TAP_FillDataBE2(0x3000, Length, buffer) != RET_OK )
    {
        core_castText("Failed to upload data");
        return 0xFFFF;
    }  

    if ( wrk_sendOneshot( HCS12_WritePC(0x2000) ) != RET_OK )
    {
        core_castText("Failed to set PC");
        return 0xFFFF;
    }

    core_castText("Running driver..");
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

    ptr = wrk_requestData( HCS12_ReadD() );
    if (!ptr)
    {
        core_castText("Could not read flash status");
        return 0xFFFF;
    }

    if (ptr[2] != 1)
    {
        core_castText("Target flagged %x as fault", ptr[2]);
        return 0xFFFF;
    }

    return RET_OK;
}

uint32_t readeepSID95(uint32_t Start, uint32_t Length)
{
    uint32_t status;
    
    wrk_ResetFault();

    if ( wrk_sendOneshot( TAP_WriteByte(0x0013,   0) ) != RET_OK )
    {
        core_castText("Failed to disable flash");
        return 0xFFFF;
    }

    if ( wrk_sendOneshot( TAP_WriteByte(0x0012,   (Start>>8) | 1 ) ) != RET_OK )
    {
        core_castText("Failed to enable EEP");
        return 0xFFFF;
    }

    core_castText("Reading eeprom");

    wrk_ResetFault();

    // Send dump request
    status = wrk_sendOneshot_NoWait(TAP_Dump(Start, Length));
    if (status != RET_OK)
        return status;
    
    while (wrk_dumpDone() == RET_BUSY)
    {
        status = busyOK();
        if (status != RET_OK)
            return status;
    }

    wrk_byteSwapBuffer(Length, 2);

    return status;

}

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Initialization functions
uint32_t initSIU95()
{
    TAP_Config_host_t config;
    uint32_t retval, Retries = 3;
    uint16_t *ptr;

    config.Type           = TAP_IO_BDMS;
    config.Frequency      = 4194000/2;
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

    // Perform a reset-stop
    retval = wrk_sendOneshot(TAP_TargetReady());
    if (retval != RET_OK)
    {
        if (Retries--)
            goto Retry;
        
    }

    waitms(500);
  
    ptr = wrk_requestData( HCS12_ReadBDMAddress(1) );
    if (!ptr) return 0xFFFF;
    core_castText("bdm1: %02X", ptr[2]);

    if ((ptr[2]&0xC0) != 0xC0 || (ptr[2]&1) || (ptr[2]&0x38))
    {
        core_castText("Device is probably in secure mode... dumps are not to be trusted!");
        return RET_OK;
    }

    ptr = wrk_requestData( HCS12_ReadBDMAddress(7) );
    if (!ptr) return 0xFFFF;
    core_castText("Base: %02X", ptr[2]);

    // Mapping to 0
    if ( wrk_sendOneshot( TAP_WriteByte((ptr[2]&0x78) + 0x11, 0x00) ) != RET_OK )
    {
        core_castText("Failed to set mode");
        return 0xFFFF;
    }

    ptr = wrk_requestData( TAP_ReadByte(0x10) );
    if (!ptr) return 0xFFFF;
    core_castText("INITRM: %04X", *(uint32_t *) &ptr[2]);


    // Map ram to 0x2000, align to lowest address
    if ( wrk_sendOneshot( TAP_WriteByte(0x0010, 0x20) ) != RET_OK )
    {
        core_castText("Failed to enable RAM");
        return 0xFFFF;
    }

    ptr = wrk_requestData( TAP_ReadWord(0x1a) );
    if (!ptr)
        return 0xFFFF;
    core_castText ("Device ID: %02x",ptr[2]);

    // Configure timeout to 0, in effect killing the watchdog
    if ( wrk_sendOneshot( TAP_WriteByte(0x003C, 0) ) != RET_OK )
    {
        core_castText("Failed disable COP");
        return 0xFFFF;
    }

    // Set SP to something useful in case we intend to use drivers later on
    if ( wrk_sendOneshot( HCS12_WriteSP(0x2FF0) ) != RET_OK )
    {
        core_castText("Failed to set SP");
        return 0xFFFF;
    }

    // Faster for ef sake...
    // This is max what the adapter can achieve more or less. 1-2~ host cycles to spare!
    // pllclk = 2 * osc * ((synr + 1) / (refdv + 1))
    // (4194000 * 2) * 2 = 16776000 pll and 8388000 bus
    config.Frequency = 4194000  * (SIDMULTI + 1) ;

    if ( wrk_sendOneshot( TAP_WriteByte(0x0034, SIDMULTI) ) != RET_OK )
    {
        core_castText("Failed to configure multiplier");
        return 0xFFFF;
    }

    if ( wrk_sendOneshot( TAP_WriteByte(0x0039, 0x80) ) != RET_OK )
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

    return retval;
}

uint32_t initBasic_HCS12()
{
    TAP_Config_host_t config;
    uint32_t retval, Retries = 3;
    uint16_t *ptr;

    config.Type           = TAP_IO_BDMS;
    config.Frequency      = 4194000/2;
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


    // Perform a reset-stop
    retval = wrk_sendOneshot(TAP_TargetReady());
    if (retval != RET_OK)
    {
        if (Retries--)
            goto Retry;
        
    }

    ptr = wrk_requestData( TAP_ReadWord(0xfffe) );
    if (!ptr) return 0xFFFF;
    // core_castText("initvector: %04X", ptr[2]);

    if ( wrk_sendOneshot( HCS12_WritePC(ptr[2]) ) != RET_OK )
    {
        core_castText("Failed to set PC");
        return 0xFFFF;
    }

    // Faster for ef sake...
    // This is max what the adapter can achieve more or less. 1-2~ host cycles to spare!
    // pllclk = 2 * osc * ((synr + 1) / (refdv + 1))
    // (4194000 * 2) * 2 = 16776000 pll and 8388000 bus
    config.Frequency = 4194000 * (SIDMULTI + 1) ;

    if ( wrk_sendOneshot( TAP_WriteByte(0x0034, SIDMULTI) ) != RET_OK )
    {
        core_castText("Failed to configure multiplier");
        return 0xFFFF;
    }

    if ( wrk_sendOneshot( TAP_WriteByte(0x0039, 0x80) ) != RET_OK )
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

    if ( wrk_sendOneshot( TAP_TargetStart() ) != RET_OK )
    {
        core_castText("Failed to start target");
        return 0xFFFF;
    }

    return retval;
}

uint32_t initSID95_MY0405()
{
    TAP_Config_host_t config;
    uint32_t retval, Retries = 3;
    uint16_t *ptr;

    config.Type           = TAP_IO_BDMS;
    config.Frequency      = 4194000/2;
    config.cfgmask.Endian = TAP_BIGENDIAN;


    // return secureEraseHCS12(4000000);
    
Retry:
    wrk_ResetFault();
    retval = wrk_sendOneshot(TAP_SetInterface(config));
    if (retval != RET_OK)
    {
        if (Retries--)
            goto Retry;
        return retval;
    }

    // Perform a reset-stop
    retval = wrk_sendOneshot(TAP_TargetReady());
    if (retval != RET_OK)
    {
        if (Retries--)
            goto Retry;
        
    }

    // waitms(500);
    ptr = wrk_requestData( HCS12_ReadBDMAddress(1) );
    if (!ptr) return 0xFFFF;
    core_castText("bdm1: %02X", ptr[2]);

    if ((ptr[2]&0xC0) != 0xC0 || (ptr[2]&1) || (ptr[2]&0x38))
    {
        core_castText("Device is probably in secure mode... dumps are not to be trusted!");
        return 0xffff;
        // return RET_OK;
    }

    ptr = wrk_requestData( HCS12_ReadBDMAddress(7) );
    if (!ptr) return 0xFFFF;
    core_castText("Base: %02X", ptr[2]);

    // Mapping to 0
    if ( wrk_sendOneshot( TAP_WriteByte((ptr[2]&0x78) + 0x11, 0x00) ) != RET_OK )
    {
        core_castText("Failed to set mode");
        return 0xFFFF;
    }

    ptr = wrk_requestData( TAP_ReadByte(0x10) );
    if (!ptr) return 0xFFFF;
    core_castText("INITRM: %04X", *(uint32_t *) &ptr[2]);


    // Map ram to 0x1000, align to lowest address
    if ( wrk_sendOneshot( TAP_WriteByte(0x0010, 0x9) ) != RET_OK )
    {
        core_castText("Failed to enable RAM");
        return 0xFFFF;
    }

    ptr = wrk_requestData( TAP_ReadWord(0x1a) );
    if (!ptr)
        return 0xFFFF;
    core_castText ("Device ID: %02x",ptr[2]);

    // Configure timeout to 0, in effect killing the watchdog
    if ( wrk_sendOneshot( TAP_WriteByte(0x003C, 0) ) != RET_OK )
    {
        core_castText("Failed disable COP");
        return 0xFFFF;
    }

    // Set SP to something useful in case we intend to use drivers later on
    if ( wrk_sendOneshot( HCS12_WriteSP(0x2FF0) ) != RET_OK )
    {
        core_castText("Failed to set SP");
        return 0xFFFF;
    }

    // Faster for ef sake...
    // This is max what the adapter can achieve more or less. 1-2~ host cycles to spare!
    // pllclk = 2 * osc * ((synr + 1) / (refdv + 1))
    // (4194000 * 2) * 2 = 16776000 pll and 8388000 bus
    config.Frequency = 4194000 * (SIDMULTI + 1) ;


    if ( wrk_sendOneshot( TAP_WriteByte(0x0034, SIDMULTI) ) != RET_OK )
    {
        core_castText("Failed to configure multiplier");
        return 0xFFFF;
    }

    if ( wrk_sendOneshot( TAP_WriteByte(0x0039, 0x80) ) != RET_OK )
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

    return retval;
}

// Guess why I had to implement this....
// It's an EXACT copy of Motorola's procedure. Excuse the weird order
uint32_t secureEraseHCS12(uint32_t clockFreq)
{
    TAP_Config_host_t config;
    uint32_t retval;
    uint16_t *ptr;
    uint8_t  tmp;

    config.Type           = TAP_IO_BDMS;
    config.Frequency      = 4000000/2;
    config.cfgmask.Endian = TAP_BIGENDIAN;

    core_castText("You fucked up, didn't you? ;)");
    core_castText("Nuking device..");

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


    if ( wrk_sendOneshot( TAP_WriteByte(0x12, 0x1) ) != RET_OK )
    {
        core_castText("Failed to enable eeprom");
        return 0xFFFF;
    }

    if ( wrk_sendOneshot( TAP_WriteByte(0x13, 0x1) ) != RET_OK )
    {
        core_castText("Failed to enable flash");
        return 0xFFFF;
    }

    if ( wrk_sendOneshot( TAP_WriteByte(0x10, 0x9) ) != RET_OK )
    {
        core_castText("Failed to enable sram");
        return 0xFFFF;
    }


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

    core_castText("Nuking eeprom");
eepwait:
    ptr = wrk_requestData( TAP_ReadByte(0x0115) );
    if (!ptr) return 0xFFFF;
    tmp = ptr[2];
    core_castText("estat: %02x", tmp&0xff);
    if ( !(tmp & 0x40) ) goto eepwait;

    core_castText("Nuking flash");
flashwait:
    ptr = wrk_requestData( TAP_ReadByte(0x0105) );
    if (!ptr) return 0xFFFF;
    tmp = ptr[2];
    core_castText("fstat: %02x", tmp&0xff);
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

    // Enable flash, eeprom, sram
    if ( wrk_sendOneshot( TAP_WriteByte(0x12, 0x1) ) != RET_OK )
    {
        core_castText("Failed to enable eeprom");
        return 0xFFFF;
    }

    if ( wrk_sendOneshot( TAP_WriteByte(0x13, 0x1) ) != RET_OK )
    {
        core_castText("Failed to enable flash");
        return 0xFFFF;
    }

    if ( wrk_sendOneshot( TAP_WriteByte(0x10, 0x9) ) != RET_OK )
    {
        core_castText("Failed to enable sram");
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

/*
    ptr = wrk_requestData( HCS12_ReadBDMAddress(1) );
    if (!ptr) return 0xFFFF;
    core_castText("bdm1: %02X", ptr[2]);
*/
    // Perform a reset-stop
    retval = wrk_sendOneshot(TAP_TargetReady());
    if (retval != RET_OK)
    {
        core_castText("Failed to ready target");
        return 0xFFFF;
    }

    waitms(500);
    core_castText("Done..?");
    return 0;    
}

// Ah.. the silence was nice. Let's restore the whine
#pragma GCC diagnostic pop

#ifdef __cplusplus 
}
#endif

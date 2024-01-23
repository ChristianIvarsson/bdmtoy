#ifdef __cplusplus 
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "targets.h"
#include "../core.h"
#include "../core_worker.h"
#include "../core_requests_CPU32.h"

#include "drivers/TxDriver.bin.h"

// SHUT UP!
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wunused-parameter"

uint32_t driverDemand(uint32_t what)
{
    uint32_t  retval = wrk_sendOneshot( CPU32_WriteDREG(0,what) );
    uint16_t *ptr;

    if (retval != RET_OK)
    {
        core_castText("Failed to write d0");
        return retval;
    }

    // Send RPC
    retval = wrk_sendOneshot( CPU32_WriteSREG(0,0x100400) );
    if (retval != RET_OK)
    {
        core_castText("Failed to send start of driver!");
        return retval;
    }

    // Run driver
    retval = wrk_sendOneshot( TAP_TargetStart() );
    if (retval != RET_OK)
    {
        core_castText("Failed to start target");
        return retval;
    }

    // Wait for target to stop
    do{ retval = TAP_TargetStatus();
    }while (retval == RET_TARGETRUNNING);
    
    if (retval != RET_TARGETSTOPPED)
    {
        core_castText("Failed to stop target");
        return retval;
    }

    // Fetch status
    ptr = wrk_requestData(CPU32_ReadDREG(0) );
    if (!ptr)
    {
        core_castText("Could not read driver status");
        return busyOK();
    }

    else if (*(uint32_t *) &ptr[2] != 1)
    {
        core_castText("Driver fail: %x", *(uint32_t *) &ptr[2]);
        return 0xFFFF;
    }

    return RET_OK;
}

uint32_t flashTrionic(uint32_t Start, uint32_t Length, const void *buffer)
{
    uint32_t  flashSize, flashType, retval;
    // uint8_t  *dataPtr  = (uint8_t *) buffer;
    uint32_t  End      = Start + Length;
    uint16_t *ptr;

    // Upload driver
    core_castText("Uploading driver..");
    retval = TAP_FillDataBE4(0x100400, sizeof(TxDriver_bin), &TxDriver_bin);
    if (retval != RET_OK) {
        core_castText("Fill failed!");
        return retval;
    }

    wrk_ConfigureTimeout(61);

    retval = driverDemand(3);
    if (retval != RET_OK)
        return retval;

    // Fetch ID
    ptr = wrk_requestData( CPU32_ReadDREG(7) );
    if (!ptr)
        return busyOK();

    core_castText("Flash MID: %02X", (ptr[2]>>8)&0xFF);
    core_castText("Flash DID: %02X", (ptr[2])&0xFF);

    // Extended id (if available)
    ptr = wrk_requestData( CPU32_ReadDREG(3) );
    if (!ptr)
        return busyOK();

    core_castText("Flash EID: %04X", (ptr[2]));

    // Flash type
    ptr = wrk_requestData( CPU32_ReadDREG(6) );
    if (!ptr)
        return busyOK();

    flashType = *(uint32_t *) &ptr[2];

    if      (flashType == 1) core_castText("Flash type: Old HW flash");
    else if (flashType == 2) core_castText("Flash type: Toggle");
    else if (flashType == 3) core_castText("Flash type: Disgusting Atmel..");
    else {
        core_castText("Internal driver fault");
        return 0xFFFF;
    }

    // Fetch size
    ptr = wrk_requestData(CPU32_ReadAREG(1) );
    if (!ptr) return busyOK();

    flashSize = *(uint32_t *) &ptr[2];

    core_castText("Flash size: %x", flashSize); 

    if (End > flashSize)
    {
        core_castText("It looks like your file is too big for the target. Aborting..");
        return 0xFFFF;
    }

    else if (End < flashSize)
    {
        while (End < flashSize)
        {
            core_castText("Doubling image %x to %x", End,End+End);
            wrk_doubleImage(End);
            Length += Length;
            End    += End;

            if (End > flashSize)
            {
                core_castText("Internal bug or weird target size. Aborting.");
                return 0xFFFF;
            }
        }
        wrk_modifyEndAddress(End);
    }

    // Erase Flash
    core_castText("Erasing flash..");
    retval = driverDemand(2);
    if (retval != RET_OK)
        return retval;

    // 
    core_castText("Writing flash..");

    // Point to start of flash (driver will increment on its own)
    retval = wrk_sendOneshot( CPU32_WriteAREG(0,0) );
    if (retval != RET_OK) {
        core_castText("Failed to write a0");
        return retval;
    }

    // Manipulate buffer for our endian
    wrk_byteSwapBuffer(End - Start, 4);

    retval = wrk_sendOneshot_NoWait(
        TAP_AssistFlash( Start, Length,
                         0x100400, 0x100000, 0x400 ) );

    if (retval != RET_OK) {
        core_castText("Failed to send AssistFlash command");
        return retval;
    }

    while (!wrk_pollFlashDone() && busyOK() == RET_OK)    { }
    retval = busyOK();
    if (retval == RET_OK)
    {
        wrk_sendOneshot( TAP_TargetReset() );
    }
    return 0;; //  busyOK();
}

uint32_t initTrionic5()
{
    TAP_Config_host_t config;
    uint32_t retval;

    config.Type           = TAP_IO_BDMOLD;
    config.Frequency      = (32768 / 4);
    config.cfgmask.Endian = TAP_BIGENDIAN;

    core_castText("Init Trionic 5x");

    wrk_ResetFault();

    wrk_newQueue( TAP_SetInterface(config) );
    wrk_queueReq( TAP_TargetReset() );
    wrk_queueReq( TAP_TargetReady() );
    wrk_queueReq( CPU32_WriteSREG(CPU32_SREG_SFC, 5) );
    wrk_queueReq( CPU32_WriteSREG(CPU32_SREG_DFC, 5) );

    wrk_queueReq( TAP_WriteWord(0xFFFA04, 0x7f00 ) ); // Exec_WriteCMD(0xFFFA04, WRITE16_BDM, 0x7f00);
    retval = wrk_sendQueue();
    if (retval != RET_OK)
    {
        return retval;
    }

    wrk_newQueue( TAP_WriteByte(0xFFFA21, 0x0000) ); // Exec_WriteCMD(0xFFFA21, WRITE8_BDM ,      0); // Watchdog
    wrk_queueReq( TAP_WriteWord(0xFFFA44, 0x3FFB) ); // Exec_WriteCMD(0xFFFA44, WRITE16_BDM, 0x3FFF); // par0
    // wrk_queueReq( TAP_WriteWord(0xFFFA46, 0x0003) ); // par1
    wrk_queueReq( TAP_WriteWord(0xFFFA48, 0x0007) ); // Exec_WriteCMD(0xFFFA48, WRITE16_BDM,    0x7); // barboot
    wrk_queueReq( TAP_WriteWord(0xFFFA4A, 0x6870) ); // Exec_WriteCMD(0xFFFA4A, WRITE16_BDM, 0x6870); // orboot
    wrk_queueReq( TAP_WriteWord(0xFFFA50, 0x0007) ); // Exec_WriteCMD(0xFFFA50, WRITE16_BDM,    0x7); // bar1
    wrk_queueReq( TAP_WriteWord(0xFFFA52, 0x3030) ); // Exec_WriteCMD(0xFFFA52, WRITE16_BDM, 0x3030); // csor1
    wrk_queueReq( TAP_WriteWord(0xFFFA54, 0x0007) ); // Exec_WriteCMD(0xFFFA54, WRITE16_BDM, 0x0007); // bar2
    wrk_queueReq( TAP_WriteWord(0xFFFA56, 0x5030) ); // Exec_WriteCMD(0xFFFA56, WRITE16_BDM, 0x5030); // csor2
    wrk_queueReq( TAP_WriteWord(0xFFFB04, 0x1000) ); // Exec_WriteCMD(0xFFFB04, WRITE16_BDM, 0x1000); // tpuram @ 10 0000
    retval = wrk_sendQueue();
    if (retval != RET_OK)
    {
        return retval;
    }

    wrk_newQueue( TAP_WriteWord(0xFFFA4C, 0x2003) ); // bar0 // SRAM at 20 0000
    wrk_queueReq( TAP_WriteWord(0xFFFA4E, 0x7870) ); // or0
    
    // PortE
    // wrk_queueReq( TAP_WriteByte(0xFFFA17, 0x00) );
    // wrk_queueReq( TAP_WriteByte(0xFFFA13, 0xA1) );
    // wrk_queueReq( TAP_WriteByte(0xFFFA15, 0xff) );

    // PortF
    wrk_queueReq( TAP_WriteByte(0xFFFA1F, 0x08) );
    wrk_queueReq( TAP_WriteByte(0xFFFA1B, 0xf8) );
    wrk_queueReq( TAP_WriteByte(0xFFFA1D, 0xf5) );

    retval = wrk_sendQueue();
    if (retval != RET_OK)
    {
        return retval;
    }

    config.Frequency = 6000000;
    return wrk_sendOneshot( TAP_SetInterface(config) );
}

// Trionic 7 - I hate you
uint32_t initTrionic7()
{
    TAP_Config_host_t config;
    uint32_t retval;

    config.Type = TAP_IO_BDMOLD;
    config.Frequency = (32768 / 4);
    config.cfgmask.Endian = TAP_BIGENDIAN;

    core_castText("Init Trionic 7");

    wrk_ResetFault();

    // Interface config
    wrk_newQueue( TAP_SetInterface(config) );
    wrk_queueReq( TAP_TargetReset() );
    wrk_queueReq( TAP_TargetReady() );
    wrk_queueReq( CPU32_WriteSREG(CPU32_SREG_SFC, 5) ); // Core CPU32 configuration
    wrk_queueReq( CPU32_WriteSREG(CPU32_SREG_DFC, 5) );
    wrk_queueReq( TAP_WriteWord(0xFFFA04, 0x7f00) );    // SYNCR - Set clock bits
    wrk_queueReq( TAP_WriteByte(0xFFFA21, 0x0000) );    // SYPCR - Set watchdog enable to 0
    if ((retval = wrk_sendQueue()) != RET_OK) { return retval; }

    ////////////////////////////////////////////////////////////////////
    // Configure chip select and base of all memory mapped, external, devices.

    wrk_newQueue( TAP_WriteWord(0xfffa44, 0x2fff) ); // CSPAR0  - cs5 as 8-bit chip select, cs4,3,2,1,0,boot as 16-bit chip select
    wrk_queueReq( TAP_WriteWord(0xfffa46, 0x0001) ); // CSPAR1  - cs6 as addr[19], The rest as discrete or alt. 339 has other hardcoded straps than 332 so hard to tell
    wrk_queueReq( TAP_WriteWord(0xfffa48, 0x0007) ); // CSBARBT - Flash. Base 00_0000, size 1M
    wrk_queueReq( TAP_WriteWord(0xfffa4a, 0x6bb0) ); // CSORBT  - Flash. MODE: Async, BYTE: Both, RW: Read , STRB: AS, DSACK: Fast, SPACE: SU/U, IPL: All, AVEC: Off
    if ((retval = wrk_sendQueue()) != RET_OK) { return retval; }

    wrk_newQueue( TAP_WriteWord(0xfffa4c, 0x2007) ); // CSBAR0  - SRAM.  Base 20_0000, size 1M
    wrk_queueReq( TAP_WriteWord(0xfffa4e, 0x6830) ); // CSOR0   - SRAM.  MODE: Async, BYTE: Both, RW: Read , STRB: AS, DSACK: 0 wt, SPACE: SU/U, IPL: All, AVEC: Off
    wrk_queueReq( TAP_WriteWord(0xfffa50, 0x0007) ); // CSBAR1  - Flash. Base 00_0000, size 1M
    wrk_queueReq( TAP_WriteWord(0xfffa52, 0x7030) ); // CSOR1   - Flash. MODE: Async, BYTE: Both, RW: Write, STRB: AS, DSACK: 0 wt, SPACE: SU/U, IPL: All, AVEC: Off
    if ((retval = wrk_sendQueue()) != RET_OK) { return retval; }

    // CSBAR2 / CSOR2 are unused (Connects to BGACK)
    // CSBAR6 / CSOR6 -> 8, 10 are unused. (addr[9], io signals etc)
    wrk_newQueue( TAP_WriteWord(0xfffa58, 0x2007) ); // CSBAR3  - SRAM.  Base 20_0000, size 1M
    wrk_queueReq( TAP_WriteWord(0xfffa5a, 0x5030) ); // CSOR3   - SRAM.  MODE: Async, BYTE: High, RW: Write, STRB: AS, DSACK: 0 wt, SPACE: SU/U, IPL: All, AVEC: Off
    wrk_queueReq( TAP_WriteWord(0xfffa5c, 0x2007) ); // CSBAR4  - SRAM.  Base 20_0000, size 1M
    wrk_queueReq( TAP_WriteWord(0xfffa5e, 0x3030) ); // CSOR4   - SRAM.  MODE: Async, BYTE: Low , RW: Write, STRB: AS, DSACK: 0 wt, SPACE: SU/U, IPL: All, AVEC: Off
    if ((retval = wrk_sendQueue()) != RET_OK) { return retval; }

    wrk_newQueue( TAP_WriteWord(0xfffa60, 0xff00) ); // CSBAR5  - CAN.   Base FF_0000, size 2k    ( CAN controller is a weird one... )
    wrk_queueReq( TAP_WriteWord(0xfffa62, 0x7bf0) ); // CSOR5   - CAN.   MODE: Async, BYTE: Both, RW: Both , STRB: AS, DSACK: Ext , SPACE: SU/U, IPL: All, AVEC: Off
    wrk_queueReq( TAP_WriteWord(0xfffa70, 0xfff8) ); // CSBAR9  - acRLY. Base FF_F800, size 2k    ( Suspect this is debug code they forgot to remove? )
    wrk_queueReq( TAP_WriteWord(0xfffa72, 0x2bc7) ); // CSOR9   - acRLY. MODE: Async, BYTE: Low , RW: Read , STRB: AS, DSACK: Ext , SPACE: CPU , IPL: 3up, AVEC: On
    if ((retval = wrk_sendQueue()) != RET_OK) { return retval; }

    wrk_newQueue( TAP_WriteWord(0xFFFB04, 0x1000) ); // TPURAM at 10_0000
    wrk_queueReq( TAP_WriteWord(0xfff706, 0x1000) ); // Latch up external power
    wrk_queueReq( TAP_WriteByte(0xfffa41, 0x001f) ); // PC0,1. CS3,4 SRAM W/E   // Configure SRAM and junk
    if ((retval = wrk_sendQueue()) != RET_OK) { return retval; }

    if (!wrk_requestData( TAP_ReadWord(0x000000) )) {
        core_castText("Unable to perform read on Flash");
        return 0xFFFF;
    }

    if (!wrk_requestData( TAP_ReadWord(0x100000) )) {
        core_castText("Unable to perform read on TPURAM");
        return 0xFFFF;
    }

    if (!wrk_requestData( TAP_ReadWord(0x200000) )) {
        core_castText("Unable to perform read on SRAM");
        return 0xFFFF;
    }

    config.Frequency = 6000000;
    return wrk_sendOneshot( TAP_SetInterface(config) );
}

uint32_t initT8main()
{
    TAP_Config_host_t config;
    uint32_t retval;

    config.Type           = TAP_IO_BDMOLD;
    config.Frequency      = 1000000;
    config.cfgmask.Endian = TAP_BIGENDIAN;

    wrk_ResetFault();

    wrk_newQueue( TAP_SetInterface(config) );
    wrk_queueReq( TAP_TargetReset() );
    wrk_queueReq( TAP_TargetReady() );
    wrk_queueReq( CPU32_WriteSREG(CPU32_SREG_SFC, 5) );
    wrk_queueReq( CPU32_WriteSREG(CPU32_SREG_DFC, 5) );

    wrk_queueReq( TAP_WriteWord(0xFFFA08, 0x7808 ) );
    retval = wrk_sendQueue();
    if (retval != RET_OK)
    {
        return retval;
    }

    wrk_newQueue( TAP_WriteWord(0xFFF6C0, 0x8400) ); // Stop FASRAM
    wrk_queueReq( TAP_WriteWord(0xFFF680, 0x8000) ); // Stop DPTRAM
    wrk_queueReq( TAP_WriteWord(0xFFF680, 0     ) ); // Start DPTRAM
    wrk_queueReq( TAP_WriteWord(0xFFF684, 0x1000) ); // ..

    // Aaaand f you, watchdog
    wrk_queueReq( TAP_WriteByte(0xFFFA27, 0x55  ) );
    wrk_queueReq( TAP_WriteByte(0xFFFA27, 0xAA  ) );
    wrk_queueReq( TAP_WriteWord(0xFFFA58, 0     ) );

    retval = wrk_sendQueue();
    if (retval != RET_OK)
    {
        return retval;
    }

    // 12 MHz is unstable!
    config.Frequency = 6000000;
    return wrk_sendOneshot( TAP_SetInterface(config) );
}

uint32_t initT8mcp()
{
    TAP_Config_host_t config;
    uint32_t retval;

    config.Type           = TAP_IO_BDMOLD;
    config.Frequency      = 1000000;
    config.cfgmask.Endian = TAP_BIGENDIAN;

    wrk_ResetFault();

    wrk_newQueue( TAP_SetInterface(config) );
    wrk_queueReq( TAP_TargetReset() );
    wrk_queueReq( TAP_TargetReady() );
    wrk_queueReq( CPU32_WriteSREG(CPU32_SREG_SFC, 5) );
    wrk_queueReq( CPU32_WriteSREG(CPU32_SREG_DFC, 5) );
    // wrk_queueReq( TAP_WriteByte(0xFFFA27,   0x55) );
    // wrk_queueReq( TAP_WriteByte(0xFFFA27,   0xAA) );
    wrk_queueReq( TAP_WriteByte(0xFFFA21,      0) ); // Watchdog..
    wrk_queueReq( TAP_WriteWord(0xFFFA04, 0xD080) ); // Clock (Used to be 0xD084)
    retval = wrk_sendQueue();
    if (retval != RET_OK)
    {
        core_castText("Failed to init MCP stage 1");
        return retval;
    }

    // Figure out if this is my contraption acting up or if MCP does not like 12 MHz (It should handle it)
    config.Frequency = 3000000;

    wrk_newQueue( TAP_SetInterface(config) );
    wrk_queueReq( TAP_WriteWord(0xFFF800, 0xC800) ); // CMFI STOP
    wrk_queueReq( TAP_WriteWord(0xFFF808,      0) ); // CMFIBAR
    wrk_queueReq( TAP_WriteWord(0xFFF80A,      0) );
    wrk_queueReq( TAP_WriteWord(0xFFF800, 0x4800) ); // CMFI enable
    wrk_queueReq( TAP_WriteWord(0xFFFB04,   0x10) ); // RAMBAR
    wrk_queueReq( TAP_WriteWord(0xFFFB06,      0) );
    wrk_queueReq( TAP_WriteWord(0xFFFB00,  0x800) ); // Ram ctl
    retval = wrk_sendQueue();

    if (retval != RET_OK)
        core_castText("Failed to init MCP stage 2");

    return retval;
}

uint32_t dumpMCP(uint32_t Start, uint32_t Length)
{
    uint32_t retval;
    uint16_t shadow;
    uint16_t *ptr;

    // Read current shadow settings
    ptr = wrk_requestData( TAP_ReadWord(0xFFF800) );
    if (!ptr)
    {
        core_castText("Failed to read current shadow setting");
        return 0xFFFF;
    }

    // Mask off to make sure shadow is disabled
    shadow = ptr[2] & 0xDFFF;

    retval = wrk_sendOneshot( TAP_WriteWord(0xFFF800, shadow) );
    if (retval != RET_OK)
    {
        core_castText("Failed to disable shadow access");
        return retval;
    }

    // Dump main block of flash
    retval = dumpGenericLE(0, 0x40000);
    if (retval != RET_OK)
    {
        core_castText("Failed to dump main block");
        return retval;
    }

    // Enable shadow access
    retval = wrk_sendOneshot( TAP_WriteWord(0xFFF800, shadow|0x2000) );
    if (retval != RET_OK)
    {
        core_castText("Failed to enable shadow access %x", retval);
        return retval;
    }

    // Dump shadow block of flash
    retval = dumpGenericLE(0, 256);
    if (retval != RET_OK)
        core_castText("Failed to dump shadow block");

    // Byteswap to the correct endian
    wrk_byteSwapBuffer(Length, 4);

    return retval;
}

uint32_t flashMCP(uint32_t Start, uint32_t Length, const void *buffer)
{
    uint32_t retval;

    // Upload driver
    core_castText("Uploading driver..");
    retval = TAP_FillDataBE4(0x100400, sizeof(TxDriver_bin), &TxDriver_bin);
    if (retval != RET_OK) {
        core_castText("Fill failed!");
        return retval;
    }

    wrk_ConfigureTimeout(61);

    // Request MCP init
    core_castText("Init flash..");
    retval = driverDemand(4);
    if (retval != RET_OK)
        return retval;

    // Point to start of flash (driver will increment on its own)
    retval = wrk_sendOneshot( CPU32_WriteAREG(0,0) );
    if (retval != RET_OK) {
        core_castText("Failed to write a0");
        return retval;
    }

    // Point to start of flash (driver will increment on its own)
    retval = wrk_sendOneshot( CPU32_WriteAREG(1,0x40100) );
    if (retval != RET_OK) {
        core_castText("Failed to write a1");
        return retval;
    }

    // Erase Flash
    core_castText("Erasing flash..");
    retval = driverDemand(2);
    if (retval != RET_OK)
        return retval;

    // 
    core_castText("Writing flash..");

    // Point to start of flash (driver will increment on its own)
    retval = wrk_sendOneshot( CPU32_WriteAREG(0,0) );
    if (retval != RET_OK) {
        core_castText("Failed to write a0");
        return retval;
    }

    // Manipulate buffer for our endian
    wrk_byteSwapBuffer(0x40100, 4);
    
    retval = wrk_sendOneshot_NoWait(
        TAP_AssistFlash( Start, Length,
                         0x100400, 0x100000, 0x400 ) );

    if (retval != RET_OK) {
        core_castText("Failed to send AssistFlash command");
        return retval;
    }

    while (!wrk_pollFlashDone() && busyOK() == RET_OK)   ;
    return busyOK();    
}

// Ah.. the silence was nice. Let's restore the whine
#pragma GCC diagnostic pop

#ifdef __cplusplus 
}
#endif

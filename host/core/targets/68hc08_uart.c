
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "../core.h"
#include "../core_worker.h"
#include "../core_requests.h"
// #include "../core_requests_JTAG.h"
// #include "../../../shared/insdef_ppc.h"








uint32_t initUARTMON()
{
    TAP_Config_host_t config;
    uint32_t retval;

    config.Type           = TAP_IO_UARTMON;
    config.Frequency      = (8000000/4)/256;
    config.cfgmask.Endian = TAP_BIGENDIAN;

    wrk_ResetFault();

    wrk_newQueue( TAP_SetInterface(config) );
    wrk_queueReq( TAP_TargetInitPort() ); // Workaround for a known problem: Force return tap to jtagc
    wrk_queueReq( TAP_TargetReset() );
    // wrk_queueReq( TAP_TargetReady() );
    retval = wrk_sendQueue();
    if (retval != RET_OK) return retval;
    
    waitms(150);
    return RET_OK;
}

uint32_t dumpUARTMON       (uint32_t Start, uint32_t Length)
{
    // uint32_t  data;
    uint16_t *ptr;
    uint32_t retval;

    wrk_newQueue( TAP_WriteByte(0x10000, 0xff) );
    wrk_queueReq( TAP_WriteByte(0x10000, 0xff) );
    wrk_queueReq( TAP_WriteByte(0x10000, 0xff) );
    wrk_queueReq( TAP_WriteByte(0x10000, 0xff) );
    wrk_queueReq( TAP_WriteByte(0x10000, 0xff) );
    wrk_queueReq( TAP_WriteByte(0x10000, 0xff) );
    wrk_queueReq( TAP_WriteByte(0x10000, 0xff) );
    wrk_queueReq( TAP_WriteByte(0x10000, 0xff) );
    retval = wrk_sendQueue();
    if (retval != RET_OK) return retval;



    waitms(150);


    retval = wrk_sendOneshot( TAP_WriteByte(0x80, 0x33) );
    if (retval != RET_OK) {
        core_castText("Could not write");
        return retval;
    }

    waitms(10);
    retval = wrk_sendOneshot( TAP_WriteByte(0x81, 0x33) );
    if (retval != RET_OK) {
        core_castText("Could not write");
        return retval;
    }
  
    waitms(10);

    ptr = wrk_requestData( TAP_ReadByte(0x80) );
    if (!ptr) {
        core_castText("Could not read FLASH_MCR");
        return 0xFFFF;
    }
    // data = ByteSwap(*(uint32_t *) &ptr[2]);
    core_castText("FLASH_MCR: %02X", ptr[2]);


    ptr = wrk_requestData( TAP_ReadByte(0x81) );
    if (!ptr) {
        core_castText("Could not read FLASH_MCR");
        return 0xFFFF;
    }
    // data = ByteSwap(*(uint32_t *) &ptr[2]);
    core_castText("FLASH_MCR: %02X", ptr[2]);

    return RET_OK;
}
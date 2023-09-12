#ifdef __cplusplus 
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "core.h"
#include "core_worker.h"

// "payload"[[cmd], [cmd + data len, words], [data (if present)]]

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Setup and toys

// This request has been changed since earlier firmwares. Sorry about that but it had to be done for future adapters
// [cmd][cmd + data len, words][type][cfg mask][speed]
void *TAP_SetInterface(TAP_Config_host_t config)
{
    static uint16_t arr[TAP_Config_sz];
    
    arr[0] = TAP_DO_SETINTERFACE;
    arr[1] = TAP_Config_sz;
    arr[2] = config.Type;

	// Microbob's POS C compiler doesn't behave nicely with packed (nor a lot of other things...)
	arr[3] = 0;
	arr[3] |= (config.cfgmask.Endian & 1) << 15;

    *(uint32_t *) &arr[4] = config.Frequency; // .."Ooops" Rules are meant to be broken!...

    return &arr[0];
}

// [cmd][cmd + data len, words]
void *TAP_TargetReady()
{
    static uint16_t arr[2];
    arr[0] = TAP_DO_TARGETREADY;
    arr[1] = 2;
    return &arr[0];
}

// [cmd][cmd + data len, words]
void *TAP_TargetInitPort()
{
    static uint16_t arr[2];
    arr[0] = TAP_DO_TARGETINITPORT;
    arr[1] = 2;
    return &arr[0];
}

// [cmd][cmd + data len, words]
void *TAP_TargetReset()
{
    static uint16_t arr[2];
    arr[0] = TAP_DO_TARGETRESET;
    arr[1] = 2;
    return &arr[0];
}

void *TAP_TargetStart()
{
    static uint16_t arr[2];
    arr[0] = TAP_DO_TARGETSTART;
    arr[1] = 2;
    return &arr[0];
}

// [cmd][cmd + data len, words]
uint32_t TAP_TargetStatus()
{
    static uint16_t arr[2];
    uint16_t *ptr;
    arr[0] = TAP_DO_TARGETSTATUS;
    arr[1] = 2;
    
    ptr = wrk_requestData( arr );
    // retval = wrk_sendOneshot( arr );
    if (!ptr)
    {
        return RET_GENERICERR;
    }
    
    return ptr[2] & 0xFFFF;
}

void *TAP_TargetRelease()
{
    static uint16_t arr[2];
    arr[0] = TAP_DO_ReleaseTarg;
    arr[1] = 2;
    return &arr[0];
}

uint32_t runDamnit() {
    return wrk_sendOneshot( TAP_TargetRelease() );
}

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Register commands

void *TAP_ReadRegister(uint16_t Reg, uint16_t Size)
{
    static uint16_t arr[4];

    arr[0] = TAP_DO_READREGISTER;
    arr[1] = 4; // Size of request in words
    arr[2] = Reg;
    arr[3] = Size;

    return &arr[0];
}

void *TAP_ReadRegByte(uint16_t Reg)
{
    static uint16_t arr[TAP_ReadReg_sz];

    arr[0] = TAP_DO_READREGISTER;
    arr[1] = TAP_ReadReg_sz;
    arr[2] = Reg;
    arr[3] = 1;

    return &arr[0];
}

void *TAP_ReadRegWord(uint16_t Reg)
{
    static uint16_t arr[TAP_ReadReg_sz];

    arr[0] = TAP_DO_READREGISTER;
    arr[1] = TAP_ReadReg_sz;
    arr[2] = Reg;
    arr[3] = 2;

    return &arr[0];
}

void *TAP_ReadRegDword(uint16_t Reg)
{
    static uint16_t arr[TAP_ReadReg_sz];

    arr[0] = TAP_DO_READREGISTER;
    arr[1] = TAP_ReadReg_sz;
    arr[2] = Reg;
    arr[3] = 4;

    return &arr[0];
}

void *TAP_WriteRegister(uint16_t Reg, uint16_t Size, uint32_t data)
{
    static uint16_t arr[6];

    arr[0] = TAP_DO_WRITEREGISTER;
    arr[1] = 4 + (Size >> 1) + (Size & 1); // Why'd you want to write for example 3 bytes..?
    arr[2] = Reg;
    arr[3] = Size;
    *(uint32_t *) &arr[4] = data;

    return &arr[0];
}

void *TAP_WriteRegByte(uint16_t Reg, uint8_t Data)
{
    static uint16_t arr[TAP_WriteReg_sz];

    arr[0] = TAP_DO_WRITEREGISTER;
    arr[1] = TAP_WriteReg_sz;
    arr[2] = Reg;
    arr[3] = 1;
    arr[4] = Data;

    return &arr[0];
}

void *TAP_WriteRegWord(uint16_t Reg, uint16_t Data)
{
    static uint16_t arr[TAP_WriteReg_sz];

    arr[0] = TAP_DO_WRITEREGISTER;
    arr[1] = TAP_WriteReg_sz;
    arr[2] = Reg;
    arr[3] = 2;
    arr[4] = Data;

    return &arr[0];
}

void *TAP_WriteRegDword(uint16_t Reg, uint32_t Data)
{
    static uint16_t arr[TAP_WriteReg_sz + 1];

    arr[0] = TAP_DO_WRITEREGISTER;
    arr[1] = TAP_WriteReg_sz + 1;
    arr[2] = Reg;
    arr[3] = 4;
    *(uint32_t *) &arr[4] = Data;

    return &arr[0];
}

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Memory; Write requests

// [addr][addr],[len][len], [data]++
void *TAP_WriteByte(uint32_t Addr, uint8_t data)
{
    static uint16_t arr[TAP_ReadCMD_sz + 1];

    arr[0] = TAP_DO_WRITEMEMORY;
    arr[1] = TAP_ReadCMD_sz + 1;

    *(uint32_t *)&arr[2] = Addr;
    *(uint32_t *)&arr[4] = 1;
                  arr[6] = data;

    return &arr[0];
}

void *TAP_WriteWord(uint32_t Addr, uint16_t data)
{
    static uint16_t arr[TAP_ReadCMD_sz + 1];
    
    arr[0] = TAP_DO_WRITEMEMORY;
    arr[1] = TAP_ReadCMD_sz + 1;

    *(uint32_t *)&arr[2] = Addr;
    *(uint32_t *)&arr[4] = 2;
                  arr[6] = data;

    return &arr[0];
}

void *TAP_WriteDword(uint32_t Addr, uint32_t data)
{
    static uint16_t arr[TAP_ReadCMD_sz + 2];

    arr[0] = TAP_DO_WRITEMEMORY;
    arr[1] = TAP_ReadCMD_sz + 2;

    *(uint32_t *)&arr[2] = Addr;
    *(uint32_t *)&arr[4] = 4;
    *(uint32_t *)&arr[6] = data;
    
    return &arr[0];
}

// [no. cmd][tot len]
// [[cmd], [cmd + data len, words]], [addr][addr],[len][len], [data]++
uint32_t TAP_FillDataBE2(uint32_t Addr, uint32_t Len, const void *dataptr)
{
    uint32_t  lenPad  = (Len&1);
    uint8_t  *dataIn  = (uint8_t *) dataptr;
    uint8_t  *dataOut = malloc(ADAPTER_BUFzIN);
    uint32_t  retval  = RET_OK;
    uint32_t  i, actToSend;
    uint8_t  *cpPtr;

    if (!dataOut)
        return wrk_faultShortcut(RET_MALLOC);
    
    *(uint16_t *) &dataOut[0] = TAP_DO_FILLMEM;

    while (Len && retval == RET_OK)
    {
        cpPtr = (uint8_t *) &dataOut[12];
        actToSend = Len+lenPad;

        // Sorry about this voodoo.. We have to take two headers into account
        if (actToSend > (ADAPTER_BUFzIN - 16))
            actToSend = (ADAPTER_BUFzIN - 16) - ((ADAPTER_BUFzIN - 16)%2);
        
        if (actToSend <= Len)
        {
            for (i = 0; i < actToSend; i++)
                cpPtr[(1-(i&1)) + (i&0xfffffffe)] = *dataIn++;
            
            cpPtr += Len;
            Len -= actToSend;
        }
        else
        {
            for (i = 0; i < Len; i++)
            {
                if (i < Len)
                    cpPtr[(1-(i&1)) + (i&0xfffffffe)] = *dataIn++;
                else
                    cpPtr[(1-(i&1)) + (i&0xfffffffe)] = 0;  
            }
            Len = 0;
        }

        // Send package..
        *(uint16_t *)&dataOut[2] = TAP_ReadCMD_sz + (actToSend/2);
        *(uint32_t *)&dataOut[4] = Addr;
        *(uint32_t *)&dataOut[8] = actToSend;
        Addr += actToSend;

        retval = wrk_sendOneshot( dataOut );
    }

    free(dataOut);

    return retval;
}

uint32_t TAP_FillDataBE4(uint32_t Addr, uint32_t Len, const void *dataptr)
{
    uint32_t  lenPad  = (Len & 3) ? (4 - (Len%4)) : 0;
    uint8_t  *dataIn  = (uint8_t *) dataptr;
    uint8_t  *dataOut = malloc(ADAPTER_BUFzIN);
    uint32_t  retval  = RET_OK;
    uint32_t  i, actToSend;
    uint8_t  *cpPtr;

    if (!dataOut)
        return wrk_faultShortcut(RET_MALLOC);
    
    *(uint16_t *) &dataOut[0] = TAP_DO_FILLMEM;

    while (Len && retval == RET_OK)
    {
        cpPtr = (uint8_t *) &dataOut[12];
        actToSend = Len+lenPad;

        // Sorry about this voodoo.. We have to take two headers into account
        if (actToSend > (ADAPTER_BUFzIN - 16))
            actToSend = (ADAPTER_BUFzIN - 16) - ((ADAPTER_BUFzIN - 16)%4);
        
        if (actToSend <= Len)
        {
            for (i = 0; i < actToSend; i++)
                cpPtr[(3-(i&3)) + (i&0xfffffffc)] = *dataIn++;
            
            cpPtr += Len;
            Len -= actToSend;
        }
        else
        {
            for (i = 0; i < Len; i++)
            {
                if (i < Len)
                    cpPtr[(3-(i&3)) + (i&0xfffffffc)] = *dataIn++;
                else
                    cpPtr[(3-(i&3)) + (i&0xfffffffc)] = 0;  
            } 
            Len = 0;
        }

        // Send package..
        *(uint16_t *)&dataOut[2] = TAP_ReadCMD_sz + (actToSend/2);
        *(uint32_t *)&dataOut[4] = Addr;
        *(uint32_t *)&dataOut[8] = actToSend;
        Addr += actToSend;

        retval = wrk_sendOneshot( dataOut );
    }

    free(dataOut);

    return retval;
}

void *TAP_AssistFlash(uint32_t Addr       , uint32_t Len,
                      uint32_t DriverStart,
                      uint32_t BufferStart, uint32_t BufferLen)
{
    static uint16_t arr[TAP_AssistCMD_sz];

    arr[0] = TAP_DO_ASSISTFLASH;
    arr[1] = TAP_AssistCMD_sz;

    *(uint32_t *)&arr[ 2] = Addr;
    *(uint32_t *)&arr[ 4] = Len;
    
    *(uint32_t *)&arr[ 6] = DriverStart;
    *(uint32_t *)&arr[ 8] = BufferStart;
    *(uint32_t *)&arr[10] = BufferLen;

    wrk_ResetflashDone();
    
    return &arr[0];
}

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Memory; Read requests

void *TAP_ReadByte(uint32_t Addr)
{
    static uint16_t data[TAP_ReadCMD_sz];
    TAP_ReadCMD_t *CMD = (TAP_ReadCMD_t *)&data[2];

    data[0] = TAP_DO_READMEMORY;
    data[1] = TAP_ReadCMD_sz;

	CMD->Address = Addr;
	CMD->Length  = 1;

    return &data[0];
}

void *TAP_ReadWord(uint32_t Addr)
{
    static uint16_t data[TAP_ReadCMD_sz];
    TAP_ReadCMD_t *CMD = (TAP_ReadCMD_t *)&data[2];

    data[0] = TAP_DO_READMEMORY;
    data[1] = TAP_ReadCMD_sz;

	CMD->Address = Addr;
	CMD->Length  = 2;

    return &data[0];
}

void *TAP_ReadDword(uint32_t Addr)
{
    static uint16_t data[TAP_ReadCMD_sz];
    TAP_ReadCMD_t *CMD = (TAP_ReadCMD_t *)&data[2];

    data[0] = TAP_DO_READMEMORY;
    data[1] = TAP_ReadCMD_sz;

	CMD->Address = Addr;
	CMD->Length  = 4;

    return &data[0];
}

void *TAP_ReadArr(uint32_t Addr, uint32_t Len)
{
    static uint16_t data[TAP_ReadCMD_sz];
    TAP_ReadCMD_t *CMD = (TAP_ReadCMD_t *)&data[2];

    data[0] = TAP_DO_READMEMORY;
    data[1] = TAP_ReadCMD_sz;

	CMD->Address = Addr;
	CMD->Length  = Len;

    return &data[0];
}

void *TAP_Dump(uint32_t Addr, uint32_t Len)
{
    static uint16_t data[TAP_ReadCMD_sz];
    TAP_ReadCMD_t *CMD = (TAP_ReadCMD_t *)&data[2];

    wrk_PatchAddress(Addr, Addr+Len);

    data[0] = TAP_DO_DUMPMEM;
    data[1] = TAP_ReadCMD_sz;

	CMD->Address = Addr;
	CMD->Length  = Len;

    return &data[0];
}

// 2+:
// [ 0 ][sizeB][instruction]             : Just execute this instruction, no returned data.
// [ 1 ][sizeB][instruction][sizeB][data]: Execute instruction and send data to target.
// [ 2 ][sizeB][instruction][sizeB]      : Execute instruction and read back data.
// [ 3 ][sizeB][instruction][sizeB][data]: Execute instruction with sent data, return received data.
void *TAP_Execute(uint64_t Ins, uint32_t iSz)
{
    static uint16_t data[8];

    if (iSz > 8 || !iSz) {
        *(uint32_t*)&data[0] = 0;
        core_castText("TAP_Execute(): Out of bounds");
        return &data[0];
    }

    data[0] = TAP_DO_ExecuteIns;
    data[1] = 4 + ( (iSz+(iSz&1)) / 2);
    data[2] = 0;

    data[3] = iSz;
    *(uint64_t*) &data[4] = Ins;

    return &data[0];
}

void *TAP_Execute_wSentData(uint64_t Ins, uint32_t iSz, uint64_t Dat, uint32_t dSz)
{
    static uint16_t data[13];
    
    if (iSz > 8 || dSz > 8 || !iSz || !dSz) {
        *(uint32_t*)&data[0] = 0;
        core_castText("TAP_Execute_wSentData(): Out of bounds");
        return &data[0];
    }

    data[0] = TAP_DO_ExecuteIns;
    data[1] = 4 + (((iSz+(iSz&1)) + (dSz+(dSz&1)))/2);
    data[2] = 1;

    data[3] = iSz;
    *(uint64_t*) &data[4] = Ins;

    data[4+((iSz+(iSz&1))/2)] = dSz;
    *(uint64_t*) &data[5+((iSz+(iSz&1))/2)] = Dat;

    return &data[0];
}

// [ 0 ] [[sizeB][instruction]] [[sizeB][PC]]                   : Just execute this instruction, no returned data.
// [ 2 ] [[sizeB][instruction]] [[sizeB retdata]] [[sizeB][PC]] : Execute instruction and read back data.
void *TAP_ExecutePC(uint64_t Ins, uint32_t iSz, uint64_t PC, uint32_t pSz)
{
    static uint16_t data[13];

    if (iSz > 8 || pSz > 8 || !iSz || !pSz) {
        *(uint32_t*)&data[0] = 0;
        core_castText("TAP_ExecutePC(): Out of bounds");
        return &data[0];
    }

    data[0] = TAP_DO_ExecuteIns;
    data[1] = 5 + (((iSz+(iSz&1)) + (pSz+(pSz&1)))/2);
    data[2] = 0;

    data[3] = iSz;
    *(uint64_t*) &data[4] = Ins;

    data[4+((iSz+(iSz&1))/2)] = pSz;
    *(uint64_t*) &data[5+((iSz+(iSz&1))/2)] = PC;

    return &data[0];
}

// [ 0 ] [[sizeB][instruction]..] [[sizeB][PC]..]                   : Just execute this instruction, no returned data.
// [ 2 ] [[sizeB][instruction]..] [[sizeB retdata]] [[sizeB][PC]..] : Execute instruction and read back data.
void *TAP_ExecutePC_wRecData(uint64_t Ins, uint32_t iSz, uint16_t dSz, uint64_t PC, uint32_t pSz)
{
    static uint16_t data[14];

    if (iSz > 8 || pSz > 8 || dSz > 8 || !iSz || !pSz || !dSz) {
        *(uint32_t*)&data[0] = 0;
        core_castText("TAP_ExecutePC_wRecData(): Out of bounds");
        return &data[0];
    }

    data[0] = TAP_DO_ExecuteIns;
    data[1] = 6 + (((iSz+(iSz&1)) + (pSz+(pSz&1)))/2);
    data[2] = 2;

    data[3] = iSz;
    *(uint64_t*) &data[4] = Ins;

    data[4+((iSz+(iSz&1))/2)] = dSz;

    data[5+((iSz+(iSz&1))/2)] = pSz;
    *(uint64_t*) &data[6+((iSz+(iSz&1))/2)] = PC;

    return &data[0];
}
#ifdef __cplusplus 
}
#endif

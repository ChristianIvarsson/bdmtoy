#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "core.h"
#include "core_worker.h"
#include "core_requests.h"
#include "targets/targets.h"
#include "utils/utils.h"

// SHUT UP!
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"

typedef void callbck (void);
typedef void castprog(int percentage);

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Private stuff: Parameters, buffers etc
#define FILEBUFFERsz (4096*1024)

static clock_t timeoutval  = GLOBALTIMEOUT;
static clock_t clock_start = 0;

// Filebuffer
static uint16_t filebuffer[FILEBUFFERsz/2];
// Temporary buffer received responses
// static uint16_t readbuffer[1024];

// Pointers to callback functions
static void *callptr = 0, *progptr = 0;
static usbSnd_t *usbSend = 0;

// Internal function pointers
static fnGen_t *initptr = 0;

static struct {
             // To adapter
             uint16_t  ReqArray[ADAPTER_BUFzIN/2];
             uint16_t *ReqPtr;
             
             // From adapter
             uint16_t  ReceivedReqArray[ADAPTER_BUFzOUT/2];
             uint16_t *ReceivedReqPtr;             
             // Shared
             uint16_t  queueLoc;
             uint16_t  queuemaster;
    volatile uint32_t  queuedcommands;
    volatile uint32_t  Busy;
    const    uint32_t  pad;
    // volatile uint32_t  wrkDone;
} wrk_queue;

static struct {
    // Fault info
    volatile uint32_t  lastFault;
    
    volatile uint32_t  workDone;
    volatile uint32_t  flashDone; // hack..
} wrk_status;

static struct {
             // Address information
             uint32_t  startAddr;
             uint32_t  expectAddr;
             uint32_t  lastAddr;

             // Frame information
             uint32_t  expectLen;
             uint32_t  multiframe;
    const    uint32_t  pad;
             uint16_t *bufpntr;
} wrk_state;


void wrk_PatchAddress(uint32_t start, uint32_t end) {
    wrk_state.startAddr  = start;
    wrk_state.expectAddr = start;
    wrk_state.lastAddr   = end;
}

static uint32_t manProg = 0;

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Private stuff: Timing

void wrk_CaptureCurreTime()
{   clock_start = clock(); }

static __inline uint32_t wrk_TimeIsTicking(const clock_t secs)
{
    clock_t current = clock();
    clock_t delta = current - clock_start;
    return ((delta / CLOCKS_PER_SEC) > secs) ? RET_TIMEOUT : RET_OK;
}

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Private stuff

// Call whoever is using this library. It's up to that code to figure out what we actually want
static void wrk_CallBack()
{
    if (callptr != 0) {
        callbck *callback = (callbck *) callptr;
        callback();
    }
}

static __inline void wrk_castProgress()
{
    uint32_t percentage = (uint32_t)((float)100 * (wrk_state.expectAddr - wrk_state.startAddr) / (float)(wrk_state.lastAddr - wrk_state.startAddr));
    static uint32_t lastProg = 0;

    // Pushing progress is kinda slow. Only do it in 5% increments
    if ((percentage > (lastProg + 4) || percentage < lastProg) && progptr != 0) {
        castprog *cast = (castprog *) progptr;
        
        if (percentage > 100)
            percentage = 100;

        cast((int32_t)percentage);
        lastProg = percentage;
    }
}

void wrk_castProgPub(float percentage)
{
    static uint32_t lastProg = 0;

    // Pushing progress is kinda slow. Only do it in 5% increments
    if ((percentage > (lastProg + 4) || percentage < lastProg) && progptr != 0) {
        castprog *cast = (castprog *) progptr;
        
        if (percentage > 100)
            percentage = 100;

        cast((int32_t)percentage);
        lastProg = (uint32_t)percentage;
    }
}

static uint32_t wrk_SendArray(void *ptr)
{
    uint16_t *arr = (uint16_t *) ptr;
    
    if (arr[0] == 0)
    {
        wrk_status.lastFault = RET_MALFORMED;
        return RET_MALFORMED;
    }
    if (usbSend != 0)
    {
        wrk_queue.Busy = 1;
        // word[1] states how many commands are queued
        wrk_queue.queuedcommands = arr[1]; // wrk_queue.ReqArray[1];
        usbSend(ptr, arr[0] * 2);
        return RET_OK;
    }
    
    wrk_status.lastFault = RET_NOPTR;
    return RET_NOPTR;
}

static uint32_t wrk_flagFault(uint32_t reason)
{    
    wrk_queue.Busy = 0;
    wrk_status.lastFault = reason;
    wrk_CallBack();
    return reason;
}

static uint32_t wrk_flagDone()
{
    wrk_queue.Busy = 0;
    wrk_status.workDone = (wrk_status.lastFault > 0) ? 0 : 1;
    wrk_CallBack();
    return RET_OK;
}

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Private stuff: Reset internals to a known state

// Outgoing queue
static void wrk_resetQueueParam()
{
    wrk_queue.ReceivedReqArray[0] = 0;
    wrk_queue.queuemaster = 0xFFFF;
    wrk_queue.queuedcommands = 0;
    wrk_queue.ReqPtr   = &wrk_queue.ReqArray[0];
    wrk_queue.Busy     =  0;
    wrk_queue.queueLoc =  0;
}

static void wrk_resetInternals(const uint32_t index)
{
    trgTemplate *targetstruc = (trgTemplate *) supportedTargets[index];
    
    wrk_state.expectLen  = 0;
    wrk_state.multiframe = 0;
    wrk_state.bufpntr    = &filebuffer[0];
    
    wrk_status.lastFault = 0;
    wrk_status.workDone  = 0;
    wrk_status.flashDone = 0;

    wrk_resetQueueParam();

    // We can't set all parameters from here..
    initptr     = targetstruc->initcode;

    timeoutval  = GLOBALTIMEOUT;
    manProg     = 0;
}

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Private stuff

//  Jump to init code if pointer has been installed
static uint32_t jmp_InitTarget()
{
    if (initptr != 0)
        return initptr();
    return RET_NOTINS;
}

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Public parameters

uint32_t wrk_pollFlashDone()
{
    return wrk_status.flashDone;
}

void wrk_doubleImage(uint32_t End)
{
    uint16_t *outPtr = (uint16_t *) &filebuffer[End/2];
    uint16_t *inPtr  = (uint16_t *) &filebuffer[0];
    uint32_t i;

    for (i = 0; i < End/2; i++)
        *outPtr++ = *inPtr++;
}

void wrk_modifyEndAddress(uint32_t End)
{
    wrk_state.lastAddr = End;
}

void wrk_byteSwapBuffer(uint32_t noBytes, uint8_t blockSize)
{
    uint8_t buffer[8];
    uint8_t *ptrbuf = (uint8_t *) &filebuffer[0];
    uint8_t *ptrtmp = (uint8_t *) &buffer[0];
    uint8_t i;

    // Dude! How do you expect me to swap something that is not in multiples of the blockSize?!
    // Do your own swapping, you buffoon...
    if (noBytes%blockSize)
        return;

    while (noBytes)
    {
        ptrbuf += blockSize;
        for (i = 0; i < blockSize; i++)
            *ptrtmp++ = *--ptrbuf;

        ptrtmp -= blockSize;
        for (i = 0; i < blockSize; i++)
            *ptrbuf++ = *ptrtmp++;

        ptrtmp  -= blockSize;
        noBytes -= blockSize;
    }
}

uint32_t busyOK()
{
    uint32_t res = wrk_status.lastFault;
    if (res != RET_OK) return res;
    return wrk_TimeIsTicking(timeoutval);
}

uint32_t wrk_dumpDone()
{
    if (wrk_state.expectAddr >= wrk_state.lastAddr)
    {
        wrk_queue.Busy = 0;
        return busyOK();
    }
    return RET_BUSY;
}

// Return 1 if work completed successfully
uint32_t core_ReturnWorkStatus()
{
    return (wrk_status.lastFault > 0) ? 0 : wrk_status.workDone;
}

// Return > 0 if any faults are stored
uint32_t core_ReturnFaultStatus()
{
    return wrk_status.lastFault;
}

// Return pointer to file buffer
const void *core_ReturnBufferPointer()
{
    if (wrk_status.workDone && !wrk_status.lastFault)
        return &filebuffer[0];
    return 0;
}

void wrk_ResetFault()
{
    wrk_status.lastFault = 0;
}

// Return number of bytes in buffer
uint32_t core_ReturnNoBytesInBuffer()
{
    if (wrk_state.startAddr > wrk_state.lastAddr || core_ReturnBufferPointer() == 0)
        return 0;
    return wrk_state.lastAddr - wrk_state.startAddr;
}

// Reveal buffer-pointer to external functions
void *hack_ReturnBufferPtr()
{
    return &filebuffer[0];
}

// You only have to raise this when using assisted functions
void wrk_ConfigureTimeout(uint32_t newVal)
{
    timeoutval = (clock_t)newVal;
}

void wrk_ResetflashDone()
{
    wrk_status.flashDone = 0;
}

void wrk_ManualProgress()
{
    manProg = 1;
}

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Installation of callback functions

void core_InstallCallback(const void *funcptr)
{
    callptr = (void *)funcptr;
}

void core_InstallProgress(const void *funcptr)
{
    progptr = (void *)funcptr;
}

void core_InstallSendArray(usbSnd_t *funcptr)
{
    usbSend = funcptr;
}

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Reception of answers / data
static __inline void wrk_setInternalFlags(uint16_t *recdata)
{    
    if (recdata[0] != 4)
        return;

    if (recdata[2] == RET_OK)
    {
        wrk_queue.Busy = 0;
        // core_castText("Driver done");
        wrk_status.flashDone = 1;
    }

    // core_castText("Adapter req'd data: %x, Len %u", bufAddr, Len);
}

static __inline void wrk_sendFlashdata(uint16_t *recdata)
{
    uint32_t  bufAddr  = *(uint32_t *) &recdata[3];
    uint32_t  Len      = *(uint32_t *) &recdata[5];
    uint16_t *flashPtr =  (uint16_t *) &filebuffer[bufAddr/2];
    uint32_t  i;

    // core_castText("Adapter req'd data: %x, Len %u", bufAddr, Len);

    // Malformed request
    if (recdata[0] != 7)
    {
        core_castText("wrk_sendFlashdata(): Malformed");
        return;
    }
    // Adapter tried to fetch more data than we have buffer!
    else if (bufAddr + Len > sizeof(filebuffer))
    {
        core_castText("wrk_sendFlashdata(): Bounds");
        return;
    }
    // McFly..?
    else if (!Len)
    {
        core_castText("wrk_sendFlashdata(): Malformed 2");
        return;
    }

    wrk_state.expectAddr = bufAddr + Len;

    if (!manProg)
        wrk_castProgress();

    // Update time since last. This'll make other functions wait indefinitely while flashing
    wrk_CaptureCurreTime();
    
    wrk_queue.ReqArray[0] = (uint16_t)((Len/2) + 3);
    wrk_queue.ReqArray[1] = 1;
    wrk_queue.ReqArray[2] = TAP_DO_ASSISTFLASH_IN;

    for (i = 0; i < (Len/2); i++ )
    {
        wrk_queue.ReqArray[3 + i] = *flashPtr++;
    }

    // 
    wrk_queue.ReceivedReqArray[0] = 0;

    // Reset queue pointer and counter
    wrk_queue.ReqPtr    = &wrk_queue.ReqArray[0];
    wrk_queue.queueLoc  =  0;

    wrk_queue.queuemaster = TAP_DO_ASSISTFLASH_IN;

    wrk_SendArray(&wrk_queue.ReqArray[0]);
}

// [tot len in words (lenword inc)] [cmd][sts] [addr][addr] [Data..]
static __inline void wrk_recDump(uint16_t *recdata)
{
    uint16_t *temppntr = (uint16_t *) &recdata[5];
    uint32_t  noWords  = recdata[0] - 5, i;

    // Come on.. You sent a header but no data?!
    if (recdata[0] < 6)
    {
        core_castText("Dump rec'd: %u (%08X : %08X)", recdata[0], *(uint32_t *) &recdata[3], wrk_state.expectAddr);
        wrk_status.lastFault = RET_MALFORMED;
        return;
    }
    
    // Uh, this is not the expected address...
    if (*(uint32_t *) &recdata[3] != wrk_state.expectAddr)
    {
        core_castText("Dump rec'd: %u (%08X : %08X)", recdata[0], *(uint32_t *) &recdata[3], wrk_state.expectAddr);
        wrk_status.lastFault = RET_FRAMEDROP;
        return;
    }

    // waitms( 50 );

    // Update time since last. This'll make other functions wait indefinitely while dumping
    wrk_CaptureCurreTime();
    
    // Copy data
    for (i = 0; i < noWords; i++)
        *wrk_state.bufpntr++ = *temppntr++;

    // Increment expected address by number of received bytes
    wrk_state.expectAddr += noWords * 2;

    wrk_castProgress();
}

// [tot len in words]  ([in response to req] [status] [len of resp in words(this word included)] [data if available])  ..
static __inline void wrk_scanQueue(uint16_t *recdata)
{
    uint16_t *strptr = (uint16_t *) &wrk_queue.ReceivedReqArray[1];
    uint16_t *ptr    = (uint16_t *) &recdata[1];
    uint32_t Len     = recdata[0];
    uint32_t noRec   = 0;
    uint16_t reqLen, status, req;
    uint32_t i;
    
    // ..
    // Stored received queue format: [tot len]  ( [req][req len(this included)][data]+ )  ..
    wrk_queue.ReceivedReqArray[0] = 0;

    // We should never receive a frame smaller than 4 words
    if (Len < 4)
    {
        core_castText("wrk_scanQueue(): Malformed");
        wrk_status.lastFault = RET_MALFORMED;
        return;
    }
    
    // ..
    Len--;

    do {
        reqLen = ptr[2];
        status = ptr[1];
        req    = ptr[0];

        // Simple boundary check, lower
        if (reqLen < 3)
        {
            core_castText("wrk_scanQueue(): reqLen < 3");
            wrk_status.lastFault = RET_MALFORMED;
            return;            
        }

        if (status != RET_OK)
        {
            core_castText("Command: %04X, failed with: %04X", req, status);
            wrk_status.lastFault = ptr[1];
            return;                
        }
        
        else
        {
            // core_castText("Command: %04X, succeeded", req);         
        }
        
        //  ([in response to req] [status] [len of resp in words(this word included)] [data if available])
        switch (req)
        {
            // Stored received queue format: [tot len]  ( [req][req len(this included)][data]+ )  ..
            case TAP_DO_READMEMORY:
            case TAP_DO_READREGISTER:
            case TAP_DO_TARGETSTATUS:
            case TAP_DO_ExecuteIns:

                // We expect these to contain some sort of data and not only a header
                if (reqLen < 4)
                {
                    // TAP_DO_ExecuteIns is quite broad and won't return data in some use-cases
                    // Catch it and just ignore for now.
                    if (req == TAP_DO_ExecuteIns) {
                        // core_castText("wrk_scanQueue(): debug, no data");
                        return;
                    }
                    wrk_status.lastFault = RET_MALFORMED;
                    return;       
                }

                // We don't store status since we know it's ok if we got here
                reqLen--;

                if (wrk_queue.ReceivedReqArray[0] == 0)
                    wrk_queue.ReceivedReqArray[0] = 1;
                
                // Add to total length of the received buffer
                wrk_queue.ReceivedReqArray[0] += reqLen;

                // Store command
                *strptr++ = req;
                
                // Store length
                *strptr++ = reqLen;
                
                // Store data..
                for (i = 0; i < (uint32_t)(reqLen - 2); i++)
                    *strptr++ = ptr[3 + i];
                break;

            default:
                break;
        }

        // Simple boundary check, upper
        if (ptr[2] > Len)
        {
            wrk_status.lastFault = RET_MALFORMED;
            return;            
        }

        Len -= ptr[2];
        ptr += ptr[2];
        noRec++;

    } while (Len);

    if (noRec != wrk_queue.queuedcommands)
    {
        core_castText("Req missmatch! s(%u) r(%u)", wrk_queue.queuedcommands, noRec);
        wrk_status.lastFault = RET_MALFORMED;
        return;     
    }
}

// Since we might receive the frame in several chunks, we first have to buffer those and then forward the whole thing to their respective owners
void core_HandleRecData(const void *bufin, uint32_t recLen)
{
    const uint16_t *recdata = (const uint16_t *) bufin;
    static uint16_t  temp[ADAPTER_BUFzOUT/2];
    static uint16_t *destptr;
    uint_fast16_t i;
    
    // core_castText("core_HandleRecData() %u", recLen);;

    // We count in words, not bytes.
    recLen /= 2;

    if (recLen == 0)
        return;

    // Not expecting anything!
    if (wrk_queue.Busy == 0)
        return;

    if (wrk_state.multiframe == 0)
    {
        destptr = temp;
        wrk_state.expectLen = recdata[0];
        
        if (wrk_state.expectLen > recLen)
            wrk_state.multiframe = 1;
    }

    // Adapter sent more data than header described.
    if (recLen > wrk_state.expectLen)
    {
        wrk_status.lastFault = RET_MALFORMED;
        return;
    }

    memcpy(destptr, bufin, recLen * 2);
    destptr += recLen;

    wrk_state.expectLen -= (wrk_state.expectLen >= recLen) ? 
        recLen :             // Only decrement number of words that was received
        wrk_state.expectLen; // No more words left

    if (wrk_state.expectLen == 0)
    {
        wrk_state.multiframe = 0;
        
        // Adapter is pissed about something. Abort and check what it wants.
        // This one will steal messages from updatestatus so won't get the whole message. I'll fix it eventually..
        if (temp[2] != RET_OK)
        {
            wrk_queue.Busy = 0;
            wrk_status.lastFault = temp[2];
            return;
        }

        switch (temp[1])
        {
            // Dump can not be queued. The rest of them, tho, can.. (The package format is also slightly different)
            case TAP_DO_DUMPMEM:
                wrk_recDump(temp);
                return;
            
            ///////////////////////////////////
            // Requests sent FROM the adapter
            case TAP_DO_ASSISTFLASH_IN:
                wrk_sendFlashdata(temp);
                return;
            
            case TAP_DO_UPDATESTATUS:
                wrk_setInternalFlags(temp);
                return;

            default:
                wrk_scanQueue(temp);
                wrk_queue.Busy = 0;
                break;
        }
    }
}

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Entry points: Target operations

void core_DumpSRAM(const uint32_t index)
{
    trgTemplate *TARGET = (trgTemplate *) supportedTargets[index];
    uint32_t status;

    if (index > core_NoTargets() || index == 0) {
        wrk_flagFault(RET_BOUNDS);
        return;
    }

    if (TARGET->dumpSram == 0) {
        wrk_flagFault(RET_NOPTR);
        return;
    }

    wrk_resetInternals(index);
    wrk_state.startAddr  = TARGET->SRAM_Address;
    wrk_state.expectAddr = TARGET->SRAM_Address;
    wrk_state.lastAddr   = TARGET->SRAM_Address + TARGET->SRAM_NoBytes;

    // Call init code
    if ((status = jmp_InitTarget()) != RET_OK) {
        wrk_flagFault(status);
        return;
    }

    // Call dump code
    if ((status = TARGET->dumpSram(TARGET->SRAM_Address, TARGET->SRAM_NoBytes)) != RET_OK) {
        wrk_flagFault(status);
        return;
    }

    wrk_flagDone();
}

void core_WriteSRAM(const uint32_t index, const void *data)
{
    trgTemplate *TARGET = (trgTemplate *) supportedTargets[index];
    uint32_t  status;

    if (index > core_NoTargets() || index == 0) {
        wrk_flagFault(RET_BOUNDS);
        return;
    }

    if (TARGET->writeSram == 0) {
        wrk_flagFault(RET_NOPTR);
        return;
    }

    wrk_resetInternals(index);
    wrk_state.startAddr  = TARGET->SRAM_Address;
    wrk_state.expectAddr = TARGET->SRAM_Address;
    wrk_state.lastAddr   = TARGET->SRAM_Address + TARGET->SRAM_NoBytes;

    // Call init code
    if ((status = jmp_InitTarget()) != RET_OK) {
        wrk_flagFault(status);
        return;
    }

    if ((TARGET->writeSram(TARGET->SRAM_Address, TARGET->SRAM_NoBytes, data)) != RET_OK) {
        wrk_flagFault(status);
        return;
    }

    wrk_flagDone();
}

void core_WriteSRAMcust(const uint32_t index, const void *data, const uint32_t address, const uint32_t len)
{
    trgTemplate *TARGET = (trgTemplate *) supportedTargets[index];
    uint32_t  status;

    if (index > core_NoTargets() || index == 0) {
        wrk_flagFault(RET_BOUNDS);
        return;
    }

    if (TARGET->writeSram == 0) {
        wrk_flagFault(RET_NOPTR);
        return;
    }

    wrk_resetInternals(index);
    wrk_state.startAddr  = (uint32_t)address;
    wrk_state.expectAddr = (uint32_t)address;
    wrk_state.lastAddr   = (uint32_t)address + len;

    // Call init code
    if ((status = jmp_InitTarget()) != RET_OK) {
        wrk_flagFault(status);
        return;
    }

    if ((TARGET->writeSram(address, len, data)) != RET_OK) {
        wrk_flagFault(status);
        return;
    }

    wrk_flagDone();
}

void core_DumpFLASH(const uint32_t index)
{
    trgTemplate *TARGET = (trgTemplate *) supportedTargets[index];
    uint32_t status;

    if (index > core_NoTargets() || index == 0)
    {
        wrk_flagFault(RET_BOUNDS);
        return;
    }

    if (TARGET->dumpFlash == 0) {
        wrk_flagFault(RET_NOPTR);
        return;
    }

    wrk_resetInternals(index);
    wrk_state.startAddr  = TARGET->FLASH_Address;
    wrk_state.expectAddr = TARGET->FLASH_Address;
    wrk_state.lastAddr   = TARGET->FLASH_Address + TARGET->FLASH_NoBytes;

    // Call init code
    if ((status = jmp_InitTarget()) != RET_OK) {
        wrk_flagFault(status);
        return;
    }

    // Call dump code
    if ((status = TARGET->dumpFlash(TARGET->FLASH_Address, TARGET->FLASH_NoBytes)) != RET_OK) {
        wrk_flagFault(status);
        return;
    }

    wrk_flagDone();
}

void core_FLASH(const uint32_t index, const void *data)
{
    trgTemplate *TARGET = (trgTemplate *) supportedTargets[index];
    uint32_t  status;

    if (index > core_NoTargets() || index == 0) {
        wrk_flagFault(RET_BOUNDS);
        return;
    }

    if (TARGET->writeFlash == 0) {
        wrk_flagFault(RET_NOPTR);
        return;
    }

    wrk_resetInternals(index);
    wrk_state.startAddr  = TARGET->FLASH_Address;
    wrk_state.expectAddr = TARGET->FLASH_Address;
    wrk_state.lastAddr   = TARGET->FLASH_Address + TARGET->FLASH_NoBytes;

    // Copy to our local buffer.
    if (TARGET->FLASH_NoBytes)
        memcpy(filebuffer, data, TARGET->FLASH_NoBytes);

    // Call init code
    if ((status = jmp_InitTarget()) != RET_OK) {
        wrk_flagFault(status);
        return;
    }

    // Call flash code
    if ((status = TARGET->writeFlash(TARGET->FLASH_Address, TARGET->FLASH_NoBytes, filebuffer)) != RET_OK) {
        wrk_flagFault(status);
        return;
    }

    wrk_flagDone();
}

void core_DumpEEPROM(const uint32_t index)
{
    trgTemplate *TARGET = (trgTemplate *) supportedTargets[index];
    uint32_t status;

    if (index > core_NoTargets() || index == 0)
    {
        wrk_flagFault(RET_BOUNDS);
        return;
    }

    if (TARGET->dumpEep == 0) {
        wrk_flagFault(RET_NOPTR);
        return;
    }

    wrk_resetInternals(index);
    wrk_state.startAddr  = TARGET->EEPROM_Address;
    wrk_state.expectAddr = TARGET->EEPROM_Address;
    wrk_state.lastAddr   = TARGET->EEPROM_Address + TARGET->EEPROM_NoBytes;

    // Call init code
    if ((status = jmp_InitTarget()) != RET_OK) {
        wrk_flagFault(status);
        return;
    }

    // Call dump code
    if ((status = TARGET->dumpEep(TARGET->EEPROM_Address, TARGET->EEPROM_NoBytes)) != RET_OK) {
        wrk_flagFault(status);
        return;
    }

    wrk_flagDone();
}

void core_WriteEEPROM(const uint32_t index, const void *data)
{
    trgTemplate *TARGET = (trgTemplate *) supportedTargets[index];
    uint32_t  status;

    if (index > core_NoTargets() || index == 0) {
        wrk_flagFault(RET_BOUNDS);
        return;
    }

    if (TARGET->writeEep == 0) {
        wrk_flagFault(RET_NOPTR);
        return;
    }

    wrk_resetInternals(index);
    wrk_state.startAddr  = TARGET->EEPROM_Address;
    wrk_state.expectAddr = TARGET->EEPROM_Address;
    wrk_state.lastAddr   = TARGET->EEPROM_Address + TARGET->EEPROM_NoBytes;

    // Copy to our local buffer.
    if (TARGET->EEPROM_NoBytes)
        memcpy(filebuffer, data, TARGET->EEPROM_NoBytes);

    // Call init code
    if ((status = jmp_InitTarget()) != RET_OK) {
        wrk_flagFault(status);
        return;
    }

    // Call flash code
    if ((status = TARGET->writeEep(TARGET->EEPROM_Address, TARGET->EEPROM_NoBytes, filebuffer)) != RET_OK) {
        wrk_flagFault(status);
        return;
    }

    wrk_flagDone();
}

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Queue operations

// TODO: Implement
// Adapter should always respond. -Even while doing things.
// If something truly goes wonkers we must wait long enough for the adapter to realize that something is wrong so it can reset internal parameters

static __inline uint32_t usualWait()
{
    wrk_CaptureCurreTime();
    while (wrk_queue.Busy && wrk_TimeIsTicking(timeoutval) == RET_OK)  {}
    
    if (wrk_queue.Busy)
        wrk_status.lastFault = RET_TIMEOUT;

    return wrk_status.lastFault;
}

// Got tired of those four-liners...
uint32_t wrk_faultShortcut(uint32_t fault) {
    wrk_status.lastFault = fault;
    return fault;
}

// To adapter:
// Header    : [total len, words], [no. sub cmds]
// Payload(s): [[cmd], [cmd + data len, words], [data (if present)]]
uint32_t wrk_newQueue(void *payloadptr)
{
    uint16_t *req = (uint16_t *) payloadptr;
    uint16_t  Len = req[1];
    uint32_t  i;
    
    // ..
    wrk_queue.ReceivedReqArray[0] = 0;

    // Reset pointer and counter
    wrk_queue.ReqPtr   = &wrk_queue.ReqArray[0];
    wrk_queue.queueLoc = 0;

    // Do not start another command until that error code has been sorted
    if (wrk_status.lastFault != RET_OK)
        return wrk_status.lastFault;

    wrk_queue.queuemaster = req[0];
    
    *wrk_queue.ReqPtr++ = req[1] + 2; // Include header in total length
    *wrk_queue.ReqPtr++ = 1;          // One queued command (so far)
    
    for (i = 0; i < Len; i++)
        *wrk_queue.ReqPtr++ = *req++;

    wrk_queue.queueLoc = Len + 2;
    return RET_OK;
}

uint32_t wrk_queueReq(void *payloadptr)
{
    uint16_t *req = (uint16_t *) payloadptr;
    uint16_t  Len = req[1];
    uint32_t  i;

    // Do not start another command until that error code has been sorted
    if (wrk_status.lastFault != RET_OK)
        return wrk_status.lastFault;

    // Hold your horses hotshot. The adapter can only do so much
    else if (wrk_queue.queueLoc + Len > 512)
        return wrk_faultShortcut(RET_OVERFLOW);

    // Never mix DUMP with anything! Not even itself
    else if (wrk_queue.queuemaster == TAP_DO_DUMPMEM || req[0] == TAP_DO_DUMPMEM)
        return wrk_faultShortcut(RET_MIXEDQUEUE);

    wrk_queue.ReqArray[0] += req[1]; // Add request length to total length
    wrk_queue.ReqArray[1] += 1;      // Increment command counter

    for (i = 0; i < Len; i++)
        *wrk_queue.ReqPtr++ = *req++;  

    wrk_queue.queueLoc += Len;
    return RET_OK;
}

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Send queue

uint32_t wrk_sendQueue()
{
    // Precaution: Reset pointer and counter for the next queue
    wrk_queue.ReqPtr   = &wrk_queue.ReqArray[0];
    wrk_queue.queueLoc = 0;
    
    // Do not start another command until that error code has been sorted
    if (wrk_status.lastFault != RET_OK)
        return wrk_status.lastFault;
    
    // Wait for old commands to complete
    else if (usualWait() != RET_OK)
        return wrk_status.lastFault;

    else if (wrk_SendArray(&wrk_queue.ReqArray[0]) != RET_OK)
        return wrk_status.lastFault;

    return usualWait();
}

uint32_t wrk_sendOneshot(void *payloadptr)
{
    uint16_t *req = (uint16_t *) payloadptr;
    uint16_t  Len = req[1];
    uint32_t  i;

    // ..
    wrk_queue.ReceivedReqArray[0] = 0;

    // Reset queue pointer
    wrk_queue.ReqPtr   = &wrk_queue.ReqArray[0];
    wrk_queue.queueLoc =  0;

    // Do not start another command until that error code has been sorted
    if (wrk_status.lastFault != RET_OK)
        return wrk_status.lastFault;

    wrk_queue.queuemaster = req[0];

    *wrk_queue.ReqPtr++ = req[1] + 2; // Include header in total length
    *wrk_queue.ReqPtr++ = 1;          // One queued command
    
    for (i = 0; i < Len; i++)
        *wrk_queue.ReqPtr++ = *req++;

    return wrk_sendQueue();
}

// Ugly hackjob...
uint32_t wrk_sendOneshot_NoWait(void *payloadptr)
{
    uint16_t *req = (uint16_t *) payloadptr;
    uint16_t  Len = req[1];
    uint32_t  i; // F*cking microBob compiler...

    // 
    wrk_queue.ReceivedReqArray[0] = 0;

    // Reset queue pointer and counter
    wrk_queue.ReqPtr    = &wrk_queue.ReqArray[0];
    wrk_queue.queueLoc  =  0;

    // Do not start another command until that error code has been sorted
    if (wrk_status.lastFault != RET_OK)
        return wrk_status.lastFault;

    wrk_queue.queuemaster = req[0];

    *wrk_queue.ReqPtr++ = req[1] + 2; // Include header in total length
    *wrk_queue.ReqPtr++ = 1;          // One queued command
    
    for (i = 0; i < Len; i++)
        *wrk_queue.ReqPtr++ = *req++;

    if (usualWait() != RET_OK)
        return wrk_status.lastFault;
    
    return wrk_SendArray(&wrk_queue.ReqArray[0]);
}

// Stored received queue format: [tot len]  ( [req][req len(this included)][data]+ )  ..
static uint16_t *wrk_returnData(uint16_t fromCmd)
{
    uint16_t *ptr      = (uint16_t *) &wrk_queue.ReceivedReqArray[1];
    uint16_t totLength = wrk_queue.ReceivedReqArray[0];
    // char     tmp[48];
    uint16_t reqLen;
    uint16_t req;

    // Wait for transfer to complete
    if (wrk_queue.Busy)
    {
        core_castText("wrk_returnData(): wrk_queue.Busy");
        return 0;
    }
    // No data received (Check last fault)
    else if (totLength == 0)
    {
        core_castText("wrk_returnData(): !totLength");
        return 0;
    }
    
    totLength--;
    
    // Scan for request
    while (totLength)
    {
        reqLen = ptr[1];
        req    = ptr[0];

        if (req == fromCmd)
        {
            // Thrash command descriptor so that data can be fetched in the same order as it was requested
            ptr[0]  = 0;
            return ptr;
        }

        if (reqLen > totLength)
            return 0;

        totLength -= reqLen;
        ptr       += reqLen;
        
    }
    
    return 0;
}

uint16_t *wrk_requestData(void *payloadptr)
{
    uint16_t *req     = (uint16_t *) payloadptr;
    uint16_t  request = req[0];
    uint16_t  Len     = req[1];
    uint32_t  i;

    // ..
    wrk_queue.ReceivedReqArray[0] = 0;

    // Reset queue pointer
    wrk_queue.ReqPtr   = &wrk_queue.ReqArray[0];
    wrk_queue.queueLoc =  0;

    // Do not start another command until that error code has been sorted
    if (wrk_status.lastFault != RET_OK)
        return 0;

    wrk_queue.queuemaster = req[0];

    *wrk_queue.ReqPtr++ = req[1] + 2; // Include header in total length
    *wrk_queue.ReqPtr++ = 1;          // One queued command
    
    for (i = 0; i < Len; i++)
        *wrk_queue.ReqPtr++ = *req++;

    if (wrk_sendQueue() != RET_OK)
        return 0;

    return wrk_returnData(request);
}

uint32_t wrk_openFile(const char *fname)
{
    size_t fileSize, actRead;
    FILE * fp  = fopen(fname, "rb");
    
    if (!fp)
        return 0;

    fseek(fp, 0L, SEEK_END);
    fileSize = ftell(fp);
    rewind(fp);

    if (fileSize == 0 || fileSize > FILEBUFFERsz)
    {
        fclose(fp);
        return 0;
    }

    actRead = fread(filebuffer, 1, fileSize, fp);

    fclose(fp);

    return (uint32_t)(actRead == fileSize) ? fileSize : 0;
}

uint32_t wrk_writeFile(const char *fname, const uint32_t noBytes)
{
    size_t actWrite;
    FILE *fp = fopen(fname, "wb");
    
    if (!fp)
        return 0xffff;

    if (noBytes == 0 || noBytes > FILEBUFFERsz)
    {
        fclose(fp);
        return 0xffff;
    }

    rewind(fp);

    actWrite = fwrite(filebuffer, 1, (size_t)noBytes, fp);

    fclose(fp);

    return (actWrite == noBytes) ? 0 : 0xffff;
}

// Ah.. the silence was nice. Let's restore the whine
#pragma GCC diagnostic pop

#ifdef __cplusplus 
}
#endif

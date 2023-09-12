#include "TAP_shared.h"

extern volatile uint32_t usbrec;

extern uint16_t receiveBuffer[ADAPTER_BUFzIN/2];
extern uint16_t sendBuffer[(ADAPTER_BUFzOUT/2)+2];
extern void     usb_receiveData();

#define HostFreq     48000000.0f

/// Template. You can pretty much use "in" however you want but "out" must ALWAYS contain these two 16-bit words:
// [0] Status. Use/create new values in enum "ReturnCodes". _NOWHERE ELSE_
// [1] Length. 2 + length of additional data (if present) in 16-bit words
typedef void DYN_Func(const uint16_t *in, uint16_t *out);

static void DUMMY_NotSupported(const uint16_t *in, uint16_t *out);
static void DUMMY_NotInstalled(const uint16_t *in, uint16_t *out);
static void TAP_SetAllNotsupported();

/// Dynamic pointers to functions.
// "TAP_ResetState" takes length into account so there is no need to patch it in case new pointers are put in here.
static struct {
    void *DYN_TargetInitPort_pntr;

    void *DYN_TargetReset_pntr;
    void *DYN_TargetReady_pntr;

    void *DYN_TargetStart_pntr;
    void *DYN_TargetStop_pntr;
    void *DYN_TargetStatus_pntr;

    void *DYN_WriteRegister_pntr;
    void *DYN_ReadRegister_pntr;

    void *DYN_WriteMemory_pntr;
    void *DYN_ReadMemory_pntr;

    void *DYN_FillMemory_pntr;
    void *DYN_DumpMemory_pntr;

    void *DYN_AssistFlash_pntr;

    void *DYN_ExecIns_pntr;
    void *DYN_RelTar_pntr;
} TAP_funcPntrs;

static struct {
    uint32_t DriveFreq;

} TAP_Configs;

/// To adapter:
// Header    : [total len, words], [no. payloads]
// Payload(s): [[cmd], [cmd + data len, words], [data (if present)]]


/// From adapter:
// Regular commands
// [total len, words], Payload[[cmd], [ status  ], [cmd len], [data (if present)]] ..next payload

// Dump OK:
// [total len, words], Payload[[cmd], [    0    ], [addr][addr], [Data..]]
// Dump Fault
// [total len, words], Payload[[cmd], [faultcode]]

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Thanks Microsoft..

typedef struct __attribute((packed)) {
    uint16_t   PAD:15;
    uint8_t Endian: 1; // Useless atm... 1: big, 0: small. Because big is the best! ;)
} cfgmask_adapter_t;
typedef struct __attribute((packed)) {
    uint16_t       Type;   // Interface type
    cfgmask_host_t cfgmask;
    uint32_t       Frequency; 
} TAP_Config_adapter_t;

void TAP_InitPins()
{
    SetPinDir(P_RDY , 2); // Ready / Freeze
    SetPinDir(P_JCMP, 0); // JTAG compat
    SetPinDir(P_TMS , 0); // Test mode select

    SetPinDir(P_TDI , 0); // Target data in  (Data from adapter)
    SetPinDir(P_TDO , 0); // Target data out (Data to adapter)
    SetPinDir(P_CLK , 0); // Clock / breakpoint

    SetPinDir(P_Trst, 0);
    // SetPinDir(P_rstcfg, 0);
}

/////////////////////////////////////////////////////////////
/// Calculate how many delay loops we need to pass nn target ticks
// inDelay: Number of host cycles already used
// targetTicks: How many ticks should pass on the target?
uint32_t TAP_calcDelay(const uint32_t inDelay, const float targetTicks, const float TargetFreq) {
    float quanta = (HostFreq * targetTicks) / TargetFreq;

    if (quanta <= inDelay) {
        return 1; // oldbdm is doing the -1 thing. Do not set it to 0
    }

    return quanta - inDelay;
}

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Assisted functions: Use these to request stuff from the host

//out: [tot len] [cmd][sts] [bufaddr][bufaddr] [Len][Len]
// in:
uint16_t *TAP_RequestData(const uint32_t Address, const uint32_t Len)
{
    uint32_t *sendPtr = (uint32_t *) &sendBuffer[3];
    sendBuffer[0] = 7;
    sendBuffer[1] = TAP_DO_ASSISTFLASH_IN;
    sendBuffer[2] = RET_OK;

    *sendPtr++ = Address;
    *sendPtr   = Len;

    usbrec = 0;
    usb_sendData(&sendBuffer[0]);

    set_Timeout(2000);
    while(!usbrec && !get_Timeout())   ;
    // disable_Timeout();

    // [total len, words], [1] [TAP_DO_ASSISTFLASH_IN] [data..]
    if (usbrec)
    {
        if (receiveBuffer[0] != (Len/2) + 3)
            return 0;
        else if (receiveBuffer[1] != 1)
            return 0;
        else if (receiveBuffer[2] != TAP_DO_ASSISTFLASH_IN)
            return 0;

        return &receiveBuffer[3];
    }

    return 0;
}

void TAP_UpdateStatus(const uint16_t status, const uint16_t flag)
{
    sendBuffer[0] = 4;
    sendBuffer[1] = TAP_DO_UPDATESTATUS;
    sendBuffer[2] = status;
    sendBuffer[3] = flag;

    usbrec = 0;
    usb_sendData(&sendBuffer[0]);
}

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Config: Set internals to selected interface

inline static void TAP_ConfigureBDM_OLD()
{
    BDMOLD_setup(TAP_Configs.DriveFreq);
    TAP_funcPntrs.DYN_TargetInitPort_pntr = &BDMOLD_InitPort;
    TAP_funcPntrs.DYN_TargetReady_pntr    = &BDMOLD_TargetReady;
    TAP_funcPntrs.DYN_TargetReset_pntr    = &BDMOLD_TargetReset;
    TAP_funcPntrs.DYN_TargetStart_pntr    = &BDMOLD_TargetStart;
    TAP_funcPntrs.DYN_TargetStop_pntr     = &BDMOLD_TargetReady;

    TAP_funcPntrs.DYN_TargetStatus_pntr   = &BDMOLD_TargetStatus;

    TAP_funcPntrs.DYN_WriteMemory_pntr    = &BDMOLD_WriteMemory;
    TAP_funcPntrs.DYN_ReadMemory_pntr     = &BDMOLD_ReadMemory;

    TAP_funcPntrs.DYN_FillMemory_pntr     = &BDMOLD_FillMemory;
    TAP_funcPntrs.DYN_DumpMemory_pntr     = &BDMOLD_DumpMemory;

    TAP_funcPntrs.DYN_WriteRegister_pntr  = &BDMOLD_WriteRegister;
    TAP_funcPntrs.DYN_ReadRegister_pntr   = &BDMOLD_ReadRegister;

    // Assisted functions
    TAP_funcPntrs.DYN_AssistFlash_pntr    = &BDMOLD_AssistFlash;
}

inline static void TAP_ConfigureBDM_HCS12()
{
    BDMHCS12_setup(TAP_Configs.DriveFreq);
    TAP_funcPntrs.DYN_TargetInitPort_pntr = &BDMHCS12_InitPort;
    TAP_funcPntrs.DYN_TargetReset_pntr    = &BDMHCS12_TargetReset;
    TAP_funcPntrs.DYN_TargetReady_pntr    = &BDMHCS12_TargetReady;
    TAP_funcPntrs.DYN_TargetStart_pntr    = &BDMHCS12_TargetStart;
    TAP_funcPntrs.DYN_RelTar_pntr         = &BDMHCS12_ReleaseTarg;

    TAP_funcPntrs.DYN_WriteMemory_pntr    = &BDMHCS12_WriteMemory;
    TAP_funcPntrs.DYN_ReadMemory_pntr     = &BDMHCS12_ReadMemory;

    TAP_funcPntrs.DYN_FillMemory_pntr     = &BDMHCS12_FillMemory;
    TAP_funcPntrs.DYN_DumpMemory_pntr     = &BDMHCS12_DumpMemory;

    TAP_funcPntrs.DYN_WriteRegister_pntr  = &BDMHCS12_WriteRegister;
    TAP_funcPntrs.DYN_ReadRegister_pntr   = &BDMHCS12_ReadRegister;
}

inline static void TAP_ConfigureJTAG()
{
    JTAG_setup(TAP_Configs.DriveFreq);
    TAP_funcPntrs.DYN_TargetInitPort_pntr = &JTAG_InitPort;
    TAP_funcPntrs.DYN_TargetReady_pntr    = &JTAG_TargetReady;
    TAP_funcPntrs.DYN_TargetReset_pntr    = &JTAG_TargetReset;

    TAP_funcPntrs.DYN_WriteRegister_pntr  = &JTAG_WriteRegister;
    TAP_funcPntrs.DYN_ReadRegister_pntr   = &JTAG_ReadRegister;
}

inline static void TAP_ConfigureNEXUS(const uint8_t generation)
{
    NEXUS_setup(TAP_Configs.DriveFreq, generation);
    TAP_funcPntrs.DYN_TargetReady_pntr    = &NEXUS_TargetReady;

    TAP_funcPntrs.DYN_WriteMemory_pntr    = &NEXUS_WriteMemory;
    TAP_funcPntrs.DYN_ReadMemory_pntr     = &NEXUS_ReadMemory;

    TAP_funcPntrs.DYN_WriteRegister_pntr  = &NEXUS_WriteRegister;
    TAP_funcPntrs.DYN_ReadRegister_pntr   = &NEXUS_ReadRegister;

    TAP_funcPntrs.DYN_ExecIns_pntr        = &NEXUS_ExecuteIns;
}

inline static void TAP_ConfigureBDM_NEW()
{
    BDMNEW_setup(TAP_Configs.DriveFreq);
    TAP_funcPntrs.DYN_TargetInitPort_pntr = &BDMNEW_InitPort;
    TAP_funcPntrs.DYN_TargetReset_pntr    = &BDMNEW_TargetReset;
    // TAP_funcPntrs.DYN_TargetReady_pntr    = &BDMNEW_TargetReady;
    TAP_funcPntrs.DYN_TargetStart_pntr    = &BDMNEW_TargetStart;
    // TAP_funcPntrs.DYN_TargetStop_pntr     = &BDMNEW_TargetReady;
    TAP_funcPntrs.DYN_TargetStatus_pntr   = &BDMNEW_TargetStatus;
    TAP_funcPntrs.DYN_RelTar_pntr         = &BDMNEW_ReleaseTarg;

    TAP_funcPntrs.DYN_WriteMemory_pntr    = &BDMNEW_WriteMemory;
    TAP_funcPntrs.DYN_ReadMemory_pntr     = &BDMNEW_ReadMemory;

    TAP_funcPntrs.DYN_FillMemory_pntr     = &BDMNEW_FillMemory;
    TAP_funcPntrs.DYN_DumpMemory_pntr     = &BDMNEW_DumpMemory;

    TAP_funcPntrs.DYN_WriteRegister_pntr  = &BDMNEW_WriteRegister;
    TAP_funcPntrs.DYN_ReadRegister_pntr   = &BDMNEW_ReadRegister;

    TAP_funcPntrs.DYN_ExecIns_pntr        = &BDMNEW_ExecuteIns;

    // Assisted functions
    TAP_funcPntrs.DYN_AssistFlash_pntr    = &BDMNEW_AssistFlash;
}

inline static void TAP_ConfigureUARTMON()
{
    UARTMON_setup(TAP_Configs.DriveFreq);
    TAP_funcPntrs.DYN_TargetInitPort_pntr = &UARTMON_InitPort;
    // TAP_funcPntrs.DYN_TargetReady_pntr    = &UARTMON_TargetReady;
    TAP_funcPntrs.DYN_TargetReset_pntr    = &UARTMON_TargetReset;
    // TAP_funcPntrs.DYN_TargetStart_pntr    = &UARTMON_TargetStart;
    // TAP_funcPntrs.DYN_TargetStop_pntr     = &UARTMON_TargetReady; // Not typo!
    // TAP_funcPntrs.DYN_TargetStatus_pntr   = &UARTMON_TargetStatus;

    TAP_funcPntrs.DYN_WriteMemory_pntr    = &UARTMON_WriteMemory;
    TAP_funcPntrs.DYN_ReadMemory_pntr     = &UARTMON_ReadMemory;
}




/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Target

inline static void TAP_TargetInitPort(const uint16_t *in, uint16_t *out) {
    if (in[0] != 2) {
        out[0] = RET_MALFORMED;
        return;
    }
    DYN_Func *TargetInitPortDyn = (DYN_Func *) TAP_funcPntrs.DYN_TargetInitPort_pntr;
    TargetInitPortDyn(&in[1], out);
}
inline static void TAP_TargetReady(const uint16_t *in, uint16_t *out) {
    if (in[0] != 2) {
        out[0] = RET_MALFORMED;
        return;
    }
    DYN_Func *TargetRdyDyn = (DYN_Func *) TAP_funcPntrs.DYN_TargetReady_pntr;
    TargetRdyDyn(&in[1], out);
}
inline static void TAP_TargetReset(const uint16_t *in, uint16_t *out) {
    if (in[0] != 2) {
        out[0] = RET_MALFORMED;
        return;
    }
    DYN_Func *TargetRstDyn = (DYN_Func *) TAP_funcPntrs.DYN_TargetReset_pntr;
    TargetRstDyn(&in[1], out);
}
inline static void TAP_TargetStart(const uint16_t *in, uint16_t *out) {
    if (in[0] != 2) {
        out[0] = RET_MALFORMED;
        return;
    }
    DYN_Func *TargetStrtDyn = (DYN_Func *) TAP_funcPntrs.DYN_TargetStart_pntr;
    TargetStrtDyn(&in[1], out);
}
inline static void TAP_TargetStop(const uint16_t *in, uint16_t *out) {
    if (in[0] != 2) {
        out[0] = RET_MALFORMED;
        return;
    }
    DYN_Func *TargetStopDyn = (DYN_Func *) TAP_funcPntrs.DYN_TargetStop_pntr;
    TargetStopDyn(&in[1], out);
}
inline static void TAP_TargetStatus(const uint16_t *in, uint16_t *out) {
    if (in[0] != 2) {
        out[0] = RET_MALFORMED;
        return;
    }
    DYN_Func *TargetStatusDyn = (DYN_Func *) TAP_funcPntrs.DYN_TargetStatus_pntr;
    TargetStatusDyn(&in[1], out);
}
/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Memory; Write requests

// [cmd len], [addr][addr],[len][len], [data]++
inline static void TAP_WriteMemory(const uint16_t *in, uint16_t *out) {
    if (in[0] < 7)
        out[0] = RET_MALFORMED;
    else if ((in[0] - 6) != (in[3]>>1) + (in[3]&1))
        out[0] = RET_MALFORMED;
    else
    {
        DYN_Func *WriteDynamic = (DYN_Func *) TAP_funcPntrs.DYN_WriteMemory_pntr;
        WriteDynamic(&in[1], out);
    }
}

// [cmd len], [addr][addr], [len][len]
inline static void TAP_FillMemory(const uint16_t *in, uint16_t *out) {
    DYN_Func *FillDynamic = (DYN_Func *) TAP_funcPntrs.DYN_FillMemory_pntr;
    FillDynamic(&in[1], out);
}

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Memory; Read requests

// [cmd len], [addr][addr],[len][len]
inline static void TAP_ReadMemory(const uint16_t *in, uint16_t *out) {
    if (in[0] != TAP_ReadCMD_sz) {
        out[0] = RET_MALFORMED;
        return;
    }
    DYN_Func *ReadDynamic = (DYN_Func *) TAP_funcPntrs.DYN_ReadMemory_pntr;
    ReadDynamic(&in[1], out);
}

// [cmd len], [addr][addr],[len][len]
inline static void TAP_DumpMemory(const uint16_t *in, uint16_t *out) {
    if (in[0] != TAP_ReadCMD_sz) {
        out[0] = RET_MALFORMED;
        return;
    }
    DYN_Func *DumpDynamic = (DYN_Func *) TAP_funcPntrs.DYN_DumpMemory_pntr;
    DumpDynamic(&in[1], out);
}

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Register commands

// [cmd len], [register][size]
inline static void TAP_ReadRegister(const uint16_t *in, uint16_t *out) {
    if (in[0] != 4)
        out[0] = RET_MALFORMED;
    else if (!in[2])
        out[0] = RET_MALFORMED;
    else {
        DYN_Func *ReadRegDyn = (DYN_Func *) TAP_funcPntrs.DYN_ReadRegister_pntr;
        ReadRegDyn(&in[1], out);
    }

}

// [cmd len], [register][size][data]++
inline static void TAP_WriteRegister(const uint16_t *in, uint16_t *out) {
    if (in[0] < 5)
        out[0] = RET_MALFORMED;
    else if (!in[2])
        out[0] = RET_MALFORMED;
    // Bricks jtag since we count bits instead of bytes..
    // else if ((in[0] - 4) != (in[2]>>1) + (in[2]&1))
    //    out[0] = RET_MALFORMED;
    else {
        DYN_Func *WriteRegDyn = (DYN_Func *) TAP_funcPntrs.DYN_WriteRegister_pntr;
        WriteRegDyn(&in[1], out);
    }
}

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Assisted commands

// [cmd len], [addr][addr],[len][len]
inline static void TAP_AssistFlash(const uint16_t *in, uint16_t *out) {
    if (in[0] != TAP_AssistCMD_sz) {
        out[0] = RET_MALFORMED;
        return;
    }
    DYN_Func *FlashDynamic = (DYN_Func *) TAP_funcPntrs.DYN_AssistFlash_pntr;
    FlashDynamic(&in[1], out);
}





/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Misc. commands

// [1+TotSizeW][ 0  ][sizeB][instruction]             : Just execute this instruction, no returned data.
// [1+TotSizeW][ 1  ][sizeB][instruction][sizeB][data]: Execute instruction and send data to target.
// [1+TotSizeW][ 2  ][sizeB][instruction][sizeB]      : Execute instruction and read back data.
// [1+TotSizeW][ 3  ][sizeB][instruction][sizeB][data]: Execute instruction with sent data, return received data.
inline static void TAP_ExecuteIns(const uint16_t *in, uint16_t *out) {

    uint16_t SizeB1 = (in[2] + (in[2]&1))/2;

    // Check Basic things first
    if (!SizeB1) {
        out[0] = RET_MALFORMED;
        return;
    }

    DYN_Func *ExecDynamic = (DYN_Func *) TAP_funcPntrs.DYN_ExecIns_pntr;
    ExecDynamic(&in[1], out);
}

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Adapter configuration; TAP

// By default, the core will release the target after a generic dump but not all target has it implemented.
// We'll just do nothing and say "ok" if it's one of those targets.
static void TAP_Releasehack(const uint16_t *in, uint16_t *out)
{ out[0] = RET_OK; }

inline static void TAP_SetInterface(const uint16_t *in, uint16_t *out)
{
    TAP_Config_adapter_t * adt = (TAP_Config_adapter_t * ) &in[1];

    // Reset internal state
    TAP_ResetState();
    TAP_SetAllNotsupported();

    // Some extra defaults (read ugly hackjobs..)
    TAP_funcPntrs.DYN_RelTar_pntr = &TAP_Releasehack;

    out[0] = RET_OK;

    if (*in != TAP_Config_sz) {
        out[0] = RET_MALFORMED;
        return;
    }

    TAP_Configs.DriveFreq = adt->Frequency;

    switch (adt->Type)
    {
        case TAP_IO_BDMOLD:
            TAP_ConfigureBDM_OLD();
            break;
        case TAP_IO_BDMNEW:
            TAP_ConfigureBDM_NEW();
            break;
        case TAP_IO_BDMS:
            TAP_ConfigureBDM_HCS12();
            break;
        case TAP_IO_UARTMON:
            TAP_ConfigureUARTMON();
            break;
        case TAP_IO_JTAG:
            TAP_ConfigureJTAG();
            break;
        case TAP_IO_NEXUS1:
            TAP_ConfigureNEXUS(1);
            break;
        case TAP_IO_NEXUS2:
            TAP_ConfigureNEXUS(2);
            break;
        case TAP_IO_NEXUS3:
            TAP_ConfigureNEXUS(3);
            break;
        default:
            out[0] = RET_NOTSUP;
            return;
    }

}

static void TAP_ReleaseTarget(const uint16_t *in, uint16_t *out) {
    if (in[0] != 2) {
        out[0] = RET_MALFORMED;
        return;
    }

    DYN_Func *ReleaseDynamic = (DYN_Func *) TAP_funcPntrs.DYN_RelTar_pntr;
    ReleaseDynamic(&in[1], out);
}
/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Failsafe

// Return not installed if code/host tries to use something that has not been set up
static void DUMMY_NotInstalled(const uint16_t *in, uint16_t *out) {
    out[0] = RET_NOTINS;
}

// future use. Return not supported if host tries to use a feature not supported by current TAP engine
static void DUMMY_NotSupported(const uint16_t *in, uint16_t *out) {
    out[0] = RET_NOTSUP;
}

inline static void TAP_ResetTAP_Configs()
{
    TAP_Configs.DriveFreq = 100000;
}

/// Reset pointers to predefined values to prevent unknown behavior.
void TAP_ResetState()
{
    // TAP_GlobalMASK   = 0;
    // TAP_CurrentOwner = 0;

    TAP_ResetTAP_Configs();

    uint32_t *pntr = (uint32_t *) &TAP_funcPntrs;
    for (uint16_t i = 0; i < (sizeof(TAP_funcPntrs) / sizeof(void *)); i++) {
        *pntr++ = (uint32_t)&DUMMY_NotInstalled;
    }
}

inline static void TAP_SetAllNotsupported() {
    uint32_t *pntr = (uint32_t *) &TAP_funcPntrs;
    for (uint16_t i = 0; i < (sizeof(TAP_funcPntrs) / sizeof(void *)); i++) {
        *pntr++ = (uint32_t)&DUMMY_NotSupported;
    }
}

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// TAP; Queue worker
void TAP_Commands(const void *bufin)
{
    uint16_t *sendbfr  = (uint16_t *) &sendBuffer;
    uint16_t *in_pntr  = (uint16_t *)  bufin;      // Incoming buffer
    uint16_t *out_pntr = (uint16_t *) &sendbfr[1]; // ..

    in_pntr++;

    uint16_t receiveNoCmds = *in_pntr++; // Fetch number of commands

    // Total length will be all the responses + 1
    sendbfr[0] = 1;

moreCommands:

    out_pntr[2] = 2;

    switch (*in_pntr)
    {
        //////////////////////////
        /// Memory; Read commands
        case TAP_DO_READMEMORY:
            TAP_ReadMemory(&in_pntr[1], &out_pntr[1]);
            break;
        case TAP_DO_DUMPMEM:
            TAP_DumpMemory(&in_pntr[1], &out_pntr[1]);
            return; // Command thrashed the outgoing buffer

        //////////////////////////
        /// Memory; Write commands
        case TAP_DO_WRITEMEMORY:
            TAP_WriteMemory(&in_pntr[1], &out_pntr[1]);
            break;
        case TAP_DO_FILLMEM:
            TAP_FillMemory(&in_pntr[1], &out_pntr[1]);
            break;

        //////////////////////////
        /// Register commands
        case TAP_DO_READREGISTER:
            TAP_ReadRegister(&in_pntr[1], &out_pntr[1]);
            break;
        case TAP_DO_WRITEREGISTER:
            TAP_WriteRegister(&in_pntr[1], &out_pntr[1]);
            break;

        //////////////////////////
        /// TAP; Configuration
        case TAP_DO_SETINTERFACE:
            TAP_SetInterface(&in_pntr[1], &out_pntr[1]);
            break;

        //////////////////////////
        /// TAP; Port operations
/*      case TAP_DO_PORTRESET:
            TAP_CallPortReset(&in_pntr[1], &out_pntr[1]);
            break;*/
        case TAP_DO_TARGETINITPORT:
            TAP_TargetInitPort(&in_pntr[1], &out_pntr[1]);
            break;
        case TAP_DO_TARGETREADY:
            TAP_TargetReady(&in_pntr[1], &out_pntr[1]);
            break;
        case TAP_DO_TARGETRESET:
            TAP_TargetReset(&in_pntr[1], &out_pntr[1]);
            break;
        case TAP_DO_TARGETSTART:
            TAP_TargetStart(&in_pntr[1], &out_pntr[1]);
            break;
        case TAP_DO_TARGETSTOP:
            TAP_TargetStop(&in_pntr[1], &out_pntr[1]);
            break;
        case TAP_DO_TARGETSTATUS:
            TAP_TargetStatus(&in_pntr[1], &out_pntr[1]);
            break;

        //////////////////////
        /// Assisted functions
        case TAP_DO_ASSISTFLASH:
            TAP_AssistFlash(&in_pntr[1], &out_pntr[1]);
            return;

        //////////////////////
        /// Misc. functions
        case TAP_DO_ExecuteIns:
            TAP_ExecuteIns(&in_pntr[1], &out_pntr[1]);
            break;

        case TAP_DO_ReleaseTarg:
            TAP_ReleaseTarget(&in_pntr[1], &out_pntr[1]);
            break;

        /// Received unknown command
        default:
            out_pntr[1] = RET_NOTSUP;
            break;
    }

    // Commands will only take their own returned data into account (in 16-bit words)
    // We add command description to the payload and as such must increment by one
    out_pntr[2]++;

    *out_pntr   = *in_pntr;    // Which command are we responding to?
    sendbfr[0] += out_pntr[2]; // Payload[[cmd], [status], _[cmd len]_, [data (if present)]] ..next payload

    // More commands?
    if (--receiveNoCmds && out_pntr[1] == RET_OK)
    {
        in_pntr  += in_pntr[1];
        out_pntr += out_pntr[2];
        goto moreCommands;
    }

    usb_sendData(&sendBuffer[0]);
}

#include "../TAP_shared.h"
#include "BDMo_private.h"

// TODO:
// BDMOLD_TargetStart_int() can get stuck if the target is acting up. Determine if the extra check is worth the penalty

// I know it's messy but it was easier to pass along delays and such this way!
typedef struct __attribute((packed)){
    uint32_t payload;       // Data in and out. Remember to shift up data to be sent since we're MSB first!

    // Delay parameters.
    uint32_t delayKeepState;

    uint32_t *pinSetPtr;
    uint32_t *pinClrPtr;
    uint32_t *pinRdPtr;

    // You want to time EXACTLY how many host cycles something took? -Look no further..
    uint32_t *dwtptr;
    uint32_t benchTime;
} oldbdm_td;

// I know it's messy but it was easier to pass along delays and such this way!
typedef struct __attribute((packed)){
    uint32_t *spiSrPntr;

    uint32_t *spiDataPntr;
    uint32_t *pinCrhPtr;

    uint32_t *pinClrPtr;
    uint32_t *pinRdPtr;

    // You want to time EXACTLY how many host cycles something took? -Look no further..
    uint32_t *dwtptr;
    uint32_t benchTime;

    uint16_t *destPtr;
    uint32_t noDwords;

} oldbdmturbo_td;

extern uint32_t BDMold_shift(oldbdm_td *params);
extern void BDMold_turbodump(oldbdmturbo_td *params);
extern void BDMold_turbofill(oldbdmturbo_td *params);
oldbdm_td testdata;
oldbdmturbo_td turbodata;

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Port manipulation; internal
static uint16_t BDMOLD_InitPort_int()
{
    CLK_HI;
    SetPinDir(P_Trst, 3);
    SetPinDir(P_CLK , 0);
    SetPinDir(P_RDY , 2); ///< Except this fella; Input, pull down
    SetPinDir(P_TDI , 0);
    SetPinDir(P_TDO , 0);
    return RET_OK;
}

static uint16_t BDMOLD_TargetReset_int()
{
    BDMOLD_InitPort_int();

    SetPinDir(P_Trst, 1);
    SetPinDir(P_CLK , 1);

    CLK_HI;
    RST_LO;
    sleep(25);

    BDMOLD_InitPort_int();

    set_Timeout(500);
    while(!RST_RD && !get_Timeout())   ;
    // disable_Timeout();

    return RST_RD ? RET_OK : RET_NOSTART;
}

static uint16_t BDMOLD_TargetReady_int()
{
    SetPinDir(P_CLK, 1);
    SetPinDir(P_TDI, 1);

    CLK_HI;
    sleep(10);
    CLK_LO;
    sleep(10);

    ///< Give it up to 500~ ms to hit the brakes..
    set_Timeout(500);
    while(!RDY_RD &&  !get_Timeout())   ;

    if(!RDY_RD)
    {
        SetPinDir(P_Trst, 1);
        RST_LO;
        sleep(25);
        SetPinDir(P_Trst, 3);

        set_Timeout(500);
        while(!RDY_RD &&  !get_Timeout())   ;
    }

    // disable_Timeout();
    return RDY_RD ? RET_OK : RET_NOTREADY;
}

static uint16_t BDMOLD_TranslateFault(const uint16_t retdata)
{
    switch (retdata) {
        case      0: return RET_MAXRETRY;
        case      1: return RET_BUSTERMERR;
        case 0xFFFF: return RET_ILLCOMMAND;
        default:     return RET_UNKERROR;
    }
}

static uint16_t BDMOLD_ExecRead(const uint16_t cmd, const uint32_t Address, uint16_t *out)
{
    uint16_t atn = 0;

    testdata.payload = cmd;
    BDMold_shift(&testdata);

    if (Address) {
        testdata.payload = Address>>16;
        BDMold_shift(&testdata);

        testdata.payload = Address;
        BDMold_shift(&testdata);
    }

    // Shift in High word
    testdata.payload = 0;
    if (cmd&0x80) {
        do{ atn = BDMold_shift(&testdata);
        } while (atn && !testdata.payload);
        out[1] = testdata.payload;
    }

    if (atn && testdata.payload)
        return BDMOLD_TranslateFault(testdata.payload);

    // Shift in Low word
    testdata.payload = 0;
    do{ atn = BDMold_shift(&testdata);
    } while (atn && !testdata.payload);
    out[0] = testdata.payload;

    return (atn && testdata.payload) ? BDMOLD_TranslateFault(testdata.payload) : RET_OK;
}

static uint16_t BDMOLD_ExecWrite(const uint16_t cmd, const uint32_t Address, const uint16_t *in)
{
    uint16_t atn = 0;

    testdata.payload = cmd;
    BDMold_shift(&testdata);

    if (Address) {
        testdata.payload = Address>>16;
        BDMold_shift(&testdata);

        testdata.payload = Address;
        BDMold_shift(&testdata);
    }

    if (cmd&0x80) {
        testdata.payload = in[1];
        BDMold_shift(&testdata);
    }

    testdata.payload = in[0];
    BDMold_shift(&testdata);

    testdata.payload = 0;
    do{ atn = BDMold_shift(&testdata);
    } while (atn && !testdata.payload);

    return (atn && testdata.payload) ? BDMOLD_TranslateFault(testdata.payload) : RET_OK;
}

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// BDM abstraction; Private

#define BDMOLD_ExecR8(a,b)  BDMOLD_ExecRead(READ8_BDM , (a), (b))
#define BDMOLD_ExecR16(a,b) BDMOLD_ExecRead(READ16_BDM, (a), (b))
#define BDMOLD_ExecR32(a,b) BDMOLD_ExecRead(READ32_BDM, (a), (b))

#define BDMOLD_ExecW8(a,b)  BDMOLD_ExecWrite(WRITE8_BDM , (a), (b))
#define BDMOLD_ExecW16(a,b) BDMOLD_ExecWrite(WRITE16_BDM, (a), (b))
#define BDMOLD_ExecW32(a,b) BDMOLD_ExecWrite(WRITE32_BDM, (a), (b))

static uint16_t BDMOLD_TargetStart_int()
{
    testdata.payload = 0;
    BDMold_shift(&testdata);

    testdata.payload = BDM_GO;
    BDMold_shift(&testdata);

    while ( RDY_RD ) ;

    return RET_OK;
}

static uint16_t BDMOLD_TargetStatus_in()
{
    uint32_t initVal = 0, stable = 0;

    while (!stable) {
        initVal = RDY_RD;
        stable  = 1;

        for (uint_fast8_t i = 0; i < 5; i++)
        {
            if (RDY_RD != initVal)
            {
                stable = 0;
                i = 5;
            }
        }
    }

    return initVal ? RET_TARGETSTOPPED : RET_TARGETRUNNING;
}

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Fill and dump functions; Private
static uint16_t BDMOLD_stuffTarget(const uint32_t Start, uint32_t Len, const uint32_t *buffer)
{
    uint8_t *tmp    = (uint8_t *) buffer;
    uint16_t retval = RET_OK;

    // To prevent infinite loop / weird data in case the host is unaware of our limitations..
    while (Len&3)
        tmp[Len++] = 0;

    retval = BDMOLD_ExecWrite(WRITE32_BDM, Start, (uint16_t *) buffer++);
    Len   -= 4;

    if (Len && retval == RET_OK)
    {
        turbodata.destPtr = (uint16_t *)buffer;
        turbodata.noDwords = (Len/4);
        BDMold_turbofill(&turbodata);
    }

    return retval;
}

static void BDMOLD_blastReceiver(uint32_t Start, const uint32_t End, uint16_t *buffer)
{
    // [tot len] [cmd][sts] [addr][addr] [Data..]
    buffer[1] = TAP_DO_DUMPMEM;
    buffer[2] = RET_OK;

    uint_fast16_t BytesToCopy = 1024;
    uint_fast16_t first  = 1;
    // uint32_t dwt = DWT->CYCCNT;
    uint32_t Start_ = Start;


    while (Start < End)
    {
        if (End-Start < BytesToCopy)
            BytesToCopy=End-Start;

        buffer[0] = (BytesToCopy / 2) + 5;
        *(uint32_t *) &buffer[3] = Start;

        uint16_t *ptr = (uint16_t *) &buffer[5];

        if ( first )
        {
            BDMOLD_ExecR32(Start, ptr);
            ptr += 2;

            // This command does everything.. out of order lol
            testdata.payload = DUMP32_BDM;
            BDMold_shift(&testdata);
        }

        turbodata.destPtr = ptr;
        turbodata.noDwords = (BytesToCopy/4) - first;
        // for (;first < BytesToCopy/4; first++)
        BDMold_turbodump(&turbodata);

        Start+= BytesToCopy;
        first = 0;

        if (usb_sendData(buffer) != RET_OK)
        {
            // printf("Failed to send!\n\r");
            buffer[0] = RET_UNKERROR;
            return;
        }
    }

    // dwt = (DWT->CYCCNT - dwt) / 48000;
    // printf("Dump took: %u ms\n",(uint16_t) dwt);

    // turbodump has a quirk that'll thrash the next command
    // So.. let's thrash this one
    BDMOLD_ExecR16(Start_, buffer);
    buffer[0] = RET_OK;
}

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Port manipulation; Public
void BDMOLD_InitPort(const uint16_t *in, uint16_t *out)
{   out[0] = BDMOLD_InitPort_int();                      }
void BDMOLD_TargetReady(const uint16_t *in, uint16_t *out)
{   out[0] = BDMOLD_TargetReady_int();                   }
void BDMOLD_TargetReset(const uint16_t *in, uint16_t *out)
{   out[0] = BDMOLD_TargetReset_int();                   }
void BDMOLD_TargetStart(const uint16_t *in, uint16_t *out)
{   out[0] = BDMOLD_TargetStart_int();                   }
void BDMOLD_TargetStatus(const uint16_t *in, uint16_t *out)
{   out[0] = RET_OK, out[1] = 3, out[2] = BDMOLD_TargetStatus_in(); }

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Internal configuration; Public
void BDMOLD_setup(const float TargetFreq)
{
    // Hardware accelerated by SPI
    spi_cfg_t cfg;
    cfg.frequency = (uint32_t)TargetFreq;
    cfg.polarity  = SPI_HIGH;
    cfg.phase     = SPI_SECOND;
    cfg.order     = SPI_MSB;
    cfg.size      = SPI_WORD;
    InitSPI(&cfg);

    turbodata.pinClrPtr   = (uint32_t *) &GPIOB->BRR;
    turbodata.pinRdPtr    = (uint32_t *) &GPIOB->IDR;
    turbodata.dwtptr      = (uint32_t *) &DWT->CYCCNT;
    turbodata.pinCrhPtr   = (uint32_t *) &GPIOB->CRH;
    turbodata.spiDataPntr = (uint32_t *) &SPI2->DR;
    turbodata.spiSrPntr   = (uint32_t *) &SPI2->SR;

    testdata.dwtptr    = (uint32_t *) &DWT->CYCCNT;
    testdata.pinSetPtr = (uint32_t *) &GPIOB->BSRR;
    testdata.pinClrPtr = (uint32_t *) &GPIOB->BRR;
    testdata.pinRdPtr  = (uint32_t *) &GPIOB->IDR;

    testdata.delayKeepState = TAP_calcDelay(24, 0.5, TargetFreq);
}

// [addr][addr],[len][len], [data]++
void BDMOLD_WriteMemory(const uint16_t *in, uint16_t *out)
{
    uint32_t Address  = *(uint32_t *) &in[0];
    uint32_t Len      = *(uint32_t *) &in[2];
    uint16_t *dataptr =  (uint16_t  *) &in[4];
    uint16_t retval   = RET_OK;

    while (Len && retval == RET_OK)
    {
        // dword
        if (Len > 3) {
            retval = BDMOLD_ExecW32(Address, dataptr);
            Len     -= 4;
            Address += 4;
            dataptr += 2;
        }
        // word
        else if (Len > 1) {
            retval = BDMOLD_ExecW16(Address, dataptr);
            Len     -= 2;
            Address += 2;
            dataptr += 1;
        }
        // byte
        else
        {
            retval = BDMOLD_ExecW8(Address, dataptr);
            Len     -= 1;
            Address += 1;
            dataptr += 1;
        }
    }

    out[0] = retval;
}

// In: [addr][addr],[len][len]
void BDMOLD_ReadMemory(const uint16_t *in, uint16_t *out)
{
    uint16_t *dataptr =  (uint16_t *) &out[2];
    uint32_t Address  = *(uint32_t *) &in[0];
    uint32_t Len      = *(uint32_t *) &in[2];
    uint16_t retval   = RET_OK;
    uint32_t noRead   = 0;

    while (noRead < Len && retval == RET_OK)
    {
        // dword
        if (Len > 3)
        {
            retval = BDMOLD_ExecR32(Address, dataptr);
            noRead  += 4;
            Address += 4;
            dataptr += 2;
        }
        // word
        else if (Len > 1)
        {
            retval = BDMOLD_ExecR16(Address, dataptr);
            noRead  += 2;
            Address += 2;
            dataptr += 1;
        }
        // byte
        else
        {
            retval = BDMOLD_ExecR8(Address, dataptr);
            *dataptr &= 0xFF;
            noRead  += 1;
            Address += 1;
            dataptr += 1;
        }
    }

    out[0] = retval;
    out[1] = 2 + (noRead>>1) + (noRead&1);
}

// [addr][addr],[len][len], [data..]
void BDMOLD_FillMemory(const uint16_t *in, uint16_t *out)
{
    TAP_ReadCMD_t *cmd = (TAP_ReadCMD_t *) in;
    out[0] = BDMOLD_stuffTarget(cmd->Address, cmd->Length, (const uint32_t *) &in[4]);
}

// [addr][addr],[len][len]
void BDMOLD_DumpMemory(const uint16_t *in, uint16_t *out)
{
    TAP_ReadCMD_t *cmd = (TAP_ReadCMD_t *) in;
    BDMOLD_blastReceiver(cmd->Address, cmd->Address + cmd->Length, out);
}

// [register][size][data]++
void BDMOLD_WriteRegister(const uint16_t *in, uint16_t *out)
{
    switch (*in & 0xFFF0)
    {
        case BDMOLD_W_DREG: // D/AREG
        case BDMOLD_W_SREG: // SREG
            // Can only write dwords!
            if (in[1] != 4) {
                out[0] = RET_UNALIGNED;
                return;
            }
            out[0] = BDMOLD_ExecWrite(*in, 0, (uint16_t *)&in[2]);
            out[1] = 2;
            return;
        default:
            out[0] = RET_NOTSUP;
            break;
    }
}

// [register][size]
void BDMOLD_ReadRegister(const uint16_t *in, uint16_t *out)
{
    switch (*in & 0xFFF0)
    {
        case BDMOLD_R_DREG: // D/AREG
        case BDMOLD_R_SREG: // SREG
            if (in[1] != 4) {
                out[0] = RET_UNALIGNED;
                return;
            }
            out[0] = BDMOLD_ExecRead(*in, 0, (uint16_t *)&out[2]);
            out[1] = 4;
            // The target only returns dwords and that is what the host gets
            return;
        default:
            out[0] = RET_NOTSUP;
            break;
    }
}

// [addr][addr],[len][len]
void BDMOLD_AssistFlash(const uint16_t *in, uint16_t *out)
{
    TAP_AssistCMD_t *cmd = (TAP_AssistCMD_t *) in;
    uint32_t Last   = cmd->Address + cmd->Length;
    uint16_t retval;
    uint32_t regRead;

    // Since we intend to thrash a bunch of buffers we better store the data of these pointers.
    uint32_t driverStart = cmd->DriverStart;
    uint32_t bufferStart = cmd->BufferStart;
    uint32_t bufferLen   = cmd->BufferLen;
    uint32_t flashStart  = cmd->Address;
    // uint32_t flashLen    = cmd->Length;

    while (flashStart < Last)
    {
        uint16_t *ptr = TAP_RequestData(flashStart, bufferLen);

        // Got no data!
        if (!ptr)
        {
            TAP_UpdateStatus(RET_NOPTR, 0);
            return;
        }

        // while running..
        set_Timeout(60000);
        retval = BDMOLD_TargetStatus_in();
        while (retval == RET_TARGETRUNNING && !get_Timeout())
            retval = BDMOLD_TargetStatus_in();

        // Timeout
        if (get_Timeout())
        {
            TAP_UpdateStatus(RET_TIMEOUT, RET_TARGETRUNNING);
            return;
        }
        // What's wrong??
        /*else if (retval != RET_TARGETSTOPPED)
        {
            // printf("Target stop fail\n\r");
            return;
        }*/
        // disable_Timeout();

        retval = BDMOLD_ExecRead(R_DREG_BDM, 0, (uint16_t *)&regRead);
        // Could not fetch status
        if (retval != RET_OK)
        {
            TAP_UpdateStatus(RET_RWERR, retval);
            return;
        }

        // Driver says something went wrong..
        else if (regRead != 1)
        {
            TAP_UpdateStatus(RET_DRIVERFAIL, retval);
            return;
        }

        retval = BDMOLD_ExecWrite(W_SREG_BDM, 0, (uint16_t *)&driverStart);
        // Could not set RPC
        if (retval != RET_OK)
        {
            TAP_UpdateStatus(RET_RWERR, retval);
            return;
        }

        retval = BDMOLD_ExecWrite(WRITE32_BDM, bufferStart, ptr);
        if (retval != RET_OK)
        {
            TAP_UpdateStatus(RET_RWERR, retval);
            return;
        }
        ptr += 2;

        turbodata.destPtr  = (uint16_t *) ptr;
        turbodata.noDwords = (bufferLen - 4) / 4;
        BDMold_turbofill(&turbodata);

        // Didn't start
        retval = BDMOLD_TargetStart_int();
        if (retval != RET_OK)
        {
            TAP_UpdateStatus(RET_NOSTART, retval);
            return;
        }

        flashStart += bufferLen;
    }

    // while running..
    set_Timeout(60000);
    retval = BDMOLD_TargetStatus_in();
    while (retval == RET_TARGETRUNNING && !get_Timeout())
        retval = BDMOLD_TargetStatus_in();

    // Timeout
    if (get_Timeout())
    {
        TAP_UpdateStatus(RET_TIMEOUT, RET_TARGETRUNNING);
        return;
    }

    // disable_Timeout();

    retval = BDMOLD_ExecRead(R_DREG_BDM, 0, (uint16_t *)&regRead);
    // Could not fetch status
    if (retval != RET_OK)
    {
        TAP_UpdateStatus(RET_RWERR, retval);
        return;
    }

    // Driver says something went wrong..
    else if (regRead != 1)
    {
        TAP_UpdateStatus(RET_DRIVERFAIL, retval);
        return;
    }

    // printf("Assisted flash complete\n\r");
    TAP_UpdateStatus(RET_OK, 0);
}

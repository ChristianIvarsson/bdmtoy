#include "../TAP_shared.h"

// How many adapter clock cycles to wait
static uint32_t jtagdelay = 0;

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Internal: lowest level

// Switch to the next state
static void EnterState(const uint8_t tms)
{
    tms ? TMS_HI : TMS_LO;
    CLK_HI;
    TAP_PreciseDelay(jtagdelay);
    CLK_LO;
}

/*
static void JTAG_ShiftRW(const uint16_t nobits, const void *in, void *out)
{
    uint32_t writedata, readdata = 0;
    const uint32_t *write_buf = in;
          uint32_t *read_buf  = out;
    uint16_t bitcnt;

    for (bitcnt = 0; bitcnt < nobits; bitcnt++)
    {
        if (!(bitcnt & 0x1f))
            writedata = *write_buf++;

        writedata&1 ? TDI_HI : TDI_LO;
        if (bitcnt == (nobits-1)) TMS_HI; // Shift-IR -> EXIT1-IR

        CLK_HI;
        TAP_PreciseDelay(jtagdelay);
        writedata >>= 1;
        readdata |= TDO_RD << (bitcnt & 0x1f);
        CLK_LO;


        if (((bitcnt & 0x1f) == 0x1f) || (bitcnt == (nobits - 1))) {
            *read_buf++ = readdata;
            readdata = 0;
        }
    }
}*/

static void JTAG_ShiftR(const uint16_t nobits, void *out)
{
    uint32_t readdata = 0;
    uint32_t *read_buf  = out;
    uint16_t bitcnt;

    TDI_LO;

    for (bitcnt = 0; bitcnt < nobits; bitcnt++)
    {
        if (bitcnt == (nobits-1)) TMS_HI; // Shift-IR -> EXIT1-IR

        CLK_HI;
        TAP_PreciseDelay(jtagdelay);
        readdata |= TDO_RD << (bitcnt & 0x1f);
        CLK_LO;

        if (((bitcnt & 0x1f) == 0x1f) || (bitcnt == (nobits - 1))) {
            *read_buf++ = readdata;
            readdata = 0;
        }
    }
}

static void JTAG_ShiftW(const uint16_t nobits, const void *in)
{
    uint32_t writedata = 0, readdata = 0;
    const uint32_t *write_buf = in;
    uint16_t bitcnt;

    for (bitcnt = 0; bitcnt < nobits; bitcnt++)
    {
        if (!(bitcnt & 0x1f))
            writedata = *write_buf++;

        writedata&1 ? TDI_HI : TDI_LO;
        if (bitcnt == (nobits-1)) TMS_HI; // Shift-IR -> EXIT1-IR

        CLK_HI;
        TAP_PreciseDelay(jtagdelay);
        writedata >>= 1;
        readdata |= TDO_RD << (bitcnt & 0x1f);
        CLK_LO;
    }
}

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Internal: low level

/*
static void JTAG_RWIREG_int(const uint16_t nobits, const void *in, void *out)
{
    EnterState(1); // Goto Select-DR-Scan
    EnterState(1); // Select-DR-Scan > Select-IR-Scan
    EnterState(0); // Select-IR-Scan > Capture-IR
    EnterState(0); // Capture-IR > Shift-IR

    JTAG_ShiftRW(nobits, in, out);

    EnterState(1); // Exit1-IR > Update-IR
    EnterState(0); // Go back to RUN-TEST/IDLE
}
static void JTAG_RWDREG_int(const uint16_t nobits, const void *in, void *out)
{
    EnterState(1); // Goto Select-DR-Scan
    EnterState(0); // Select-DR-Scan > Capture-DR
    EnterState(0); // Capture-DR > Shift-DR

    JTAG_ShiftRW(nobits, in, out);

    EnterState(1); // Exit1-DR > Update-DR
    EnterState(0); // Update-DR > Run-Test/Idle
}*/

static void JTAG_ReadIREG_int(const uint16_t nobits, void *out)
{
    EnterState(1); // Goto Select-DR-Scan
    EnterState(1); // Select-DR-Scan > Select-IR-Scan
    EnterState(0); // Select-IR-Scan > Capture-IR
    EnterState(0); // Capture-IR > Shift-IR

    JTAG_ShiftR(nobits, out);

    EnterState(1); // Exit1-IR > Update-IR
    EnterState(0); // Go back to RUN-TEST/IDLE
}

static void JTAG_WriteIREG_int(const uint16_t nobits, const void *in)
{
    EnterState(1); // Goto Select-DR-Scan
    EnterState(1); // Select-DR-Scan > Select-IR-Scan
    EnterState(0); // Select-IR-Scan > Capture-IR
    EnterState(0); // Capture-IR > Shift-IR

    JTAG_ShiftW(nobits, in);

	EnterState(1); // Exit1-IR > Update-IR
	EnterState(0); // Go back to RUN-TEST/IDLE
}

static void JTAG_ReadDREG_int(const uint16_t nobits, void *out)
{
    EnterState(1); // Goto Select-DR-Scan
    EnterState(0); // Select-DR-Scan > Capture-DR
    EnterState(0); // Capture-DR > Shift-DR

    JTAG_ShiftR(nobits, out);

    EnterState(1); // Exit1-DR > Update-DR
    EnterState(0); // Update-DR > Run-Test/Idle
}

static void JTAG_WriteDREG_int(const uint16_t nobits, const void *in)
{
    EnterState(1); // Goto Select-DR-Scan
    EnterState(0); // Select-DR-Scan > Capture-DR
    EnterState(0); // Capture-DR > Shift-DR

    JTAG_ShiftW(nobits, in);

    EnterState(1); // Exit1-DR > Update-DR
    EnterState(0); // Update-DR > Run-Test/Idle
}



// Reset test logic to a known state
static uint16_t JTAG_ResetState_int()
{
    // Set state to 1 more than enough times and you'll end up in "TEST LOGIC RESET"
    for (uint_fast8_t i = 0; i < 128; i++)
        EnterState(1);

    EnterState(0); // Switch to RUN-TEST/IDLE
    EnterState(0); // Re-enter the same state

    return RET_OK;
}

// Hardware reset
static uint16_t JTAG_Reset_int()
{
    SetPinDir(P_Trst, 1);
    RST_LO;
    sleep(5);

    SetPinDir(P_CLK , 1);
    SetPinDir(P_TDI , 1);
    SetPinDir(P_TDO , 0);
    SetPinDir(P_TMS , 1);
    SetPinDir(P_JCMP, 1); // JTAG compat for targets that need it

    JCM_HI;
    TDI_LO; // We sample on rising edge..

    SetPinDir(P_Trst, 3);

    sleep(500);

    return JTAG_ResetState_int();
}

static uint16_t JTAG_InitPort_int()
{
    SetPinDir(P_Trst, 3);
    SetPinDir(P_CLK , 0);
    SetPinDir(P_TDI , 0);
    SetPinDir(P_TDO , 0);
    SetPinDir(P_TMS , 0);
    SetPinDir(P_JCMP, 0);

    sleep(50);

    return RET_OK;
}
/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Public

void JTAG_setup(const float TargetFreq, const uint16_t prescaler)
{   jtagdelay = TAP_calcDelay(16, 1, TargetFreq); }
void JTAG_InitPort(const uint16_t *in, uint16_t *out)
{   out[0] = JTAG_InitPort_int(); }
void JTAG_TargetReset(const uint16_t *in, uint16_t *out) // Target reset / set pins to the correct state
{   out[0] = JTAG_Reset_int(); }
void JTAG_TargetReady(const uint16_t *in, uint16_t *out) // Reset tap to a known state
{   out[0] = JTAG_ResetState_int(); }

// [register][size][data]++
void JTAG_WriteRegister(const uint16_t *in, uint16_t *out)
{
    out[0] = RET_OK;

    switch (*in)
    {
        case JTAG_IREG:
            JTAG_WriteIREG_int(in[1], &in[2]);
            return;
        case JTAG_DREG:
            JTAG_WriteDREG_int(in[1], &in[2]);
            return;
        default:
            out[0] = RET_NOTSUP;
            break;
    }
}

// [register][size]
void JTAG_ReadRegister(const uint16_t *in, uint16_t *out)
{
    out[0] = RET_OK;
    out[1] = 2 + (in[1]>>4) + ((in[1]&0xF) ? 1 : 0);

    switch (*in)
    {
        case JTAG_IREG:
            JTAG_ReadIREG_int(in[1], &out[2]);
            return;
        case JTAG_DREG:
            JTAG_ReadDREG_int(in[1], &out[2]);
            return;
        default:
            out[0] = RET_NOTSUP;
            break;
    }
}

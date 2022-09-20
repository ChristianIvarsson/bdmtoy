#include "../TAP_shared.h"
#include "NEXUS_private.h"

// Strict aliasing rules... Please call nine whine whine.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"

#define ACCESS_AUX_TAP_ONCE 0x11

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Internal: Shared stuff

static uint32_t nexusdelay   = 0;
static uint8_t  nexusmaxgen  = 1;
static uint8_t  nexuscurrgen = 1;

// OnCE status register bits. Automatically fetched whenever possible
static struct __attribute((packed)) {
    uint8_t     PAD:3; // bit[9] is one due to regulations, the other two are reserved, set to 0.
    uint8_t   DEBUG:1; // Debug mode. Set once the CPU is in debug mode. It is negated once the CPU exits debug mode (even during a debug session).
    uint8_t    STOP:1; // Stop mode. Reflects the logic level on the CPU p_stopped output after capture by j_tclk
    uint8_t    HALT:1; // Halt mode. Reflects the logic level on the CPU p_halted output after capture by j_tclk.
    uint8_t   RESET:1; // Reset mode. Reflects the inverted logic level on the CPU p_reset_b input after capture by j_tclk.
    uint8_t CHKSTOP:1; // Checkstop mode. Reflects the logic level on the CPU p_chkstop output after capture by j_tclk.
    uint8_t     ERR:1; // Error encountered during last single-step?
    uint8_t    MCLK:1; // m_clk status bit. Reflects the logic level on the jd_mclk_on input signal after capture by j_tclk
} OnCE_OSR;

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Internal: lowest level

// Switch to next state
static void NEXUS_State(const uint8_t tms)
{
    tms ? TMS_HI : TMS_LO;
    CLK_HI;
    TAP_PreciseDelay(nexusdelay);
    CLK_LO;
}

// Fall back to jtag
static uint16_t NEXUS_ResetState()
{
    // Set state to 1 more than enough times and you'll end up in "TEST LOGIC RESET"
    for (uint_fast8_t i = 0; i < 65; i++)
        NEXUS_State(1);

    NEXUS_State(0); // Switch to RUN-TEST/IDLE
    NEXUS_State(0); // Re-enter the same state

    return RET_OK;
}

// Shift in AND out data
static void NEXUS_ShiftRW(const uint16_t bitnr, const void *write, void *read)
{
    const uint32_t *writeptr = write;
    uint32_t *readptr  = read;
    uint32_t writedata, readdata = 0;
    uint16_t bitcnt;

    for (bitcnt = 0; bitcnt < bitnr; bitcnt++) {

        if (!(bitcnt & 0x1f))
            writedata = *writeptr++;

        writedata&1 ? TDI_HI : TDI_LO;
        if (bitcnt == (bitnr-1)) TMS_HI; // Shift-IR -> EXIT1-IR

        CLK_HI;
        TAP_PreciseDelay(nexusdelay);
        readdata |= TDO_RD << (bitcnt & 0x1f);
        writedata >>= 1;
        CLK_LO;

        if (((bitcnt & 0x1f) == 0x1f) || (bitcnt == (bitnr - 1))) {
            *readptr++ = readdata;
            readdata = 0;
        }
    }
}

// Shift in data
static void NEXUS_ShiftR(const uint16_t bitnr, void *read)
{
    uint32_t *readptr  = read;
    uint32_t readdata = 0;
    uint16_t bitcnt;

    TDI_LO;

    for (bitcnt = 0; bitcnt < bitnr; bitcnt++) {

        if (bitcnt == (bitnr-1)) TMS_HI; // Shift-IR -> EXIT1-IR

        CLK_HI;
        TAP_PreciseDelay(nexusdelay);
        readdata |= TDO_RD << (bitcnt & 0x1f);
        CLK_LO;

        if (((bitcnt & 0x1f) == 0x1f) || (bitcnt == (bitnr - 1))) {
            // printf("NEXUS shift read: %08x\n", readdata);
            *readptr++ = readdata;
            readdata = 0;
        }
    }
}

// Shift out data
static void NEXUS_ShiftW(const uint16_t bitnr, const void *write)
{
    const uint32_t *writeptr = write;
    uint32_t writedata = 0;
    uint16_t bitcnt;

    for (bitcnt = 0; bitcnt < bitnr; bitcnt++) {

        if (!(bitcnt & 0x1f))
            writedata = *writeptr++;

        writedata&1 ? TDI_HI : TDI_LO;
        if (bitcnt == (bitnr-1)) TMS_HI; // Shift-IR -> EXIT1-IR

        CLK_HI;
        TAP_PreciseDelay(nexusdelay);
        writedata >>= 1;
        CLK_LO;
    }
}

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Internal: low level

// Write DR
static void NEXUS_WriteDREG(const uint16_t bitnr, const void *in)
{
    NEXUS_State(1); // > SELECT-DR-SCAN
    NEXUS_State(0); // > CAPTURE-DR
    NEXUS_State(0); // > SHIFT-DR

    NEXUS_ShiftW(bitnr, in);

    NEXUS_State(1); // > UPDATE-DR
    NEXUS_State(0); // > RUN-TEST/IDLE
}

/// Read DR
static void NEXUS_ReadDREG(const uint16_t bitnr, void *out)
{
    NEXUS_State(1); // > SELECT-DR-SCAN
    NEXUS_State(0); // > CAPTURE-DR
    NEXUS_State(0); // > SHIFT-DR

    NEXUS_ShiftR(bitnr, out);

    NEXUS_State(1); // > UPDATE-DR
    NEXUS_State(0); // > RUN-TEST/IDLE
}


static void NEXUS_ReadIREG(const uint16_t nobits, void *out)
{
    NEXUS_State(1); // Goto Select-DR-Scan
    NEXUS_State(1); // Select-DR-Scan > Select-IR-Scan
    NEXUS_State(0); // Select-IR-Scan > Capture-IR
    NEXUS_State(0); // Capture-IR > Shift-IR

    NEXUS_ShiftR(nobits, out);

    NEXUS_State(1); // Exit1-IR > Update-IR
    NEXUS_State(0); // Go back to RUN-TEST/IDLE
}

static void NEXUS_WriteIREG(const uint16_t nobits, const void *in)
{
    NEXUS_State(1); // Goto Select-DR-Scan
    NEXUS_State(1); // Select-DR-Scan > Select-IR-Scan
    NEXUS_State(0); // Select-IR-Scan > Capture-IR
    NEXUS_State(0); // Capture-IR > Shift-IR

    NEXUS_ShiftW(nobits, in);

    NEXUS_State(1); // Exit1-IR > Update-IR
    NEXUS_State(0); // Go back to RUN-TEST/IDLE
}


/// Kind of a macro. Send command to DR, update register and then send data for data command
static void NEXUS3_WriteCommand(const uint8_t reg, const void *in)
{
    uint8_t data = reg << 1 | 1;
    NEXUS_WriteDREG(8, &data);
    NEXUS_WriteDREG(32, in);
}

static void NEXUS3_ReadCommand(const uint8_t reg, void *out)
{
    uint8_t data = reg << 1;
    NEXUS_WriteDREG(8, &data);
    NEXUS_ReadDREG(32, out);
}

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Internal: Abstraction

/////////////////////////////////////////////////////////////
// NEXUS 1: OCMD

// Perform the actual OCMD write
static void NEXUS1_WriteOCMD(OnCE_OCMD_t OCMD)
{
    NEXUS_State(1); // RUN-TEST/IDLE  -> SELECT-DR-SCAN
    NEXUS_State(1); // SELECT-DR-SCAN -> SELECT-IR-SCAN
    NEXUS_State(0); // SELECT-IR-SCAN -> CAPTURE-IR
    NEXUS_State(0); // CAPTURE-IR     -> SHIFT-IR

    NEXUS_ShiftRW(10, &OCMD, &OnCE_OSR);

    NEXUS_State(1); // EXIT1-IR  -> UPDATE-IR
    NEXUS_State(0); // UPDATE-IR -> RUN-TEST/IDLE
}

/*
static void NEXUS1_ReadOSR()
{
    OnCE_OCMD_t OCMD;
    *(uint16_t *) &OCMD = 0;
    OCMD.RW = 1; // 0 = write, 1 = read
    OCMD.RS = OnCE_CMD_NOREGSEL;
    NEXUS1_WriteOCMD(OCMD);
}*/

///////////////////////////////////////
// "Macro" functions to help out with other tasks
static void NEXUS1_ReadOCMDRegister(uint8_t reg, uint16_t noTransfers, void *out)
{
    OnCE_OCMD_t  OCMD;
    OCMD.RS = reg & 0x7f;
    OCMD.EX = 0;
    OCMD.GO = 0;
    OCMD.RW = 1; // 1 = Read, 0 = Write

    NEXUS1_WriteOCMD(OCMD);
    NEXUS_ReadDREG(32*noTransfers, out);

    // Apparently this register has to be read..
    uint32_t tmp;
    OCMD.RS = OnCE_CMD_NOREGSEL;
    NEXUS1_WriteOCMD(OCMD);
    NEXUS_ReadDREG(32, &tmp);
}

static void NEXUS1_WriteOCMDRegister(uint8_t reg, uint16_t noTransfers, const void *in)
{
    OnCE_OCMD_t  OCMD;
    OCMD.RS = reg & 0x7f;
    OCMD.EX = 0;
    OCMD.GO = 0;
    OCMD.RW = 0; // 1 = Read, 0 = Write

    NEXUS1_WriteOCMD(OCMD);
    NEXUS_WriteDREG(32*noTransfers, in);

    // Apparently this register has to be read..
    uint32_t tmp;
    OCMD.RW = 1; // 1 = Read, 0 = Write
    OCMD.RS = OnCE_CMD_NOREGSEL;
    NEXUS1_WriteOCMD(OCMD);
    NEXUS_ReadDREG(32, &tmp);
}

/////////////////////////////////////////////////////////////
// NEXUS 1: MISC: Abstraction for host computer

static uint16_t NEXUS1_ReadCPUID(void *out)
{
    NEXUS1_ReadOCMDRegister(OnCE_CMD_JTAG_ID, 1, out);
    return RET_OK;
}


/////////////////////////////////////////////////////////////
// NEXUS 1: OCMD: Abstraction for host computer

static uint16_t NEXUS1_ReadCPUSCR(void *out)
{
    NEXUS1_ReadOCMDRegister(OnCE_CMD_CPUSCR, 6, out);
    return RET_OK;
}

static uint16_t NEXUS1_WriteCPUSCR(const void *in)
{
    NEXUS1_WriteOCMDRegister(OnCE_CMD_CPUSCR, 6, in);
    return RET_OK;
}

static uint16_t NEXUS1_ReadCPUSCR_CTL(void *out)
{
    uint32_t *data = out;
    OnCE_CPUSCR_t CPUSCR;

    NEXUS1_ReadOCMDRegister(OnCE_CMD_CPUSCR, 6, &CPUSCR);
    *data = CPUSCR.CTL;

    return RET_OK;
}

static uint16_t NEXUS1_WriteCPUSCR_CTL(const void *in)
{
    const uint32_t *data = in;
    OnCE_CPUSCR_t CPUSCR;

    NEXUS1_ReadOCMDRegister(OnCE_CMD_CPUSCR, 6, &CPUSCR);
    CPUSCR.CTL = *data;

    NEXUS1_WriteOCMDRegister(OnCE_CMD_CPUSCR, 6, &CPUSCR);

    return RET_OK;
}

static uint16_t NEXUS1_ReadCPUSCR_IR(void *out)
{
    uint32_t *data = out;
    OnCE_CPUSCR_t CPUSCR;

    NEXUS1_ReadOCMDRegister(OnCE_CMD_CPUSCR, 6, &CPUSCR);
    *data = CPUSCR.IR;

    return RET_OK;
}

static uint16_t NEXUS1_WriteCPUSCR_IR(const void *in)
{
    const uint32_t *data = in;
    OnCE_CPUSCR_t CPUSCR;

    NEXUS1_ReadOCMDRegister(OnCE_CMD_CPUSCR, 6, &CPUSCR);
    CPUSCR.IR = *data;

    NEXUS1_WriteOCMDRegister(OnCE_CMD_CPUSCR, 6, &CPUSCR);

    return RET_OK;
}

static uint16_t NEXUS1_ReadCPUSCR_PC(void *out)
{
    uint32_t *data = out;
    OnCE_CPUSCR_t CPUSCR;

    NEXUS1_ReadOCMDRegister(OnCE_CMD_CPUSCR, 6, &CPUSCR);
    *data = CPUSCR.PC;

    return RET_OK;
}

static uint16_t NEXUS1_WriteCPUSCR_PC(const void *in)
{
    const uint32_t *data = in;
    OnCE_CPUSCR_t CPUSCR;

    NEXUS1_ReadOCMDRegister(OnCE_CMD_CPUSCR, 6, &CPUSCR);
    CPUSCR.PC = *data;

    NEXUS1_WriteOCMDRegister(OnCE_CMD_CPUSCR, 6, &CPUSCR);

    return RET_OK;
}

static uint16_t NEXUS1_ReadCPUSCR_MSR(void *out)
{
    uint32_t *data = out;
    OnCE_CPUSCR_t CPUSCR;

    NEXUS1_ReadOCMDRegister(OnCE_CMD_CPUSCR, 6, &CPUSCR);
    *data = CPUSCR.MSR;

    return RET_OK;
}

static uint16_t NEXUS1_WriteCPUSCR_MSR(const void *in)
{
    const uint32_t *data = in;
    OnCE_CPUSCR_t CPUSCR;

    NEXUS1_ReadOCMDRegister(OnCE_CMD_CPUSCR, 6, &CPUSCR);
    CPUSCR.MSR = *data;

    NEXUS1_WriteOCMDRegister(OnCE_CMD_CPUSCR, 6, &CPUSCR);

    return RET_OK;
}

static uint16_t NEXUS1_ReadCPUSCR_WBBRUpper(void *out)
{
    uint32_t *data = out;
    OnCE_CPUSCR_t CPUSCR;

    NEXUS1_ReadOCMDRegister(OnCE_CMD_CPUSCR, 6, &CPUSCR);
    *data = CPUSCR.WBBRUpper;

    return RET_OK;
}

static uint16_t NEXUS1_WriteCPUSCR_WBBRUpper(const void *in)
{
    const uint32_t *data = in;
    OnCE_CPUSCR_t CPUSCR;

    NEXUS1_ReadOCMDRegister(OnCE_CMD_CPUSCR, 6, &CPUSCR);
    CPUSCR.WBBRUpper = *data;

    NEXUS1_WriteOCMDRegister(OnCE_CMD_CPUSCR, 6, &CPUSCR);

    return RET_OK;
}

static uint16_t NEXUS1_ReadCPUSCR_WBBRLower(void *out)
{
    uint32_t *data = out;
    OnCE_CPUSCR_t CPUSCR;

    NEXUS1_ReadOCMDRegister(OnCE_CMD_CPUSCR, 6, &CPUSCR);
    *data = CPUSCR.WBBRLower;

    return RET_OK;
}

static uint16_t NEXUS1_WriteCPUSCR_WBBRLower(const void *in)
{
    const uint32_t *data = in;
    OnCE_CPUSCR_t CPUSCR;

    NEXUS1_ReadOCMDRegister(OnCE_CMD_CPUSCR, 6, &CPUSCR);
    CPUSCR.WBBRLower = *data;

    NEXUS1_WriteOCMDRegister(OnCE_CMD_CPUSCR, 6, &CPUSCR);

    return RET_OK;
}

/////////////////////////////////////////////////////////////
// NEXUS 3

static uint16_t NEXUS3_FetchData(const uint8_t bitSz, uint16_t Count, void *out)
{
    NEUXS_RWCS_t RWCS;
    *(uint32_t *) &RWCS = 0;
    uint8_t *outptr = out;
    uint8_t data    = 0xA << 1;
    uint8_t quantSz = bitSz / 8;

    RWCS.AC  = 1;            // Start access
    RWCS.SZ  = quantSz >> 1; // 0 = 8, 1 = 16, 2 = 32
    RWCS.CNT = Count;        // NoAccess
    NEXUS3_WriteCommand(7, &RWCS);  // Configure RWCS register
    NEXUS3_ReadCommand(7, &RWCS);   // Read back, check status bits
    if (RWCS.ERR)
    {
        // printf("NEXUS3_FetchData: ERR!\n");
        return RWCS.DV ? RET_RESTRICMEM : RET_GENERICERR;
    }

    while (Count--)
    {
        NEXUS_State(1); // > SELECT-DR-SCAN
        NEXUS_State(0); // > CAPTURE-DR
        NEXUS_State(0); // > SHIFT-DR

        NEXUS_ShiftW(8, &data);

        NEXUS_State(1); // > UPDATE-DR
        NEXUS_State(1); // > SELECT-DR-SCAN
        NEXUS_State(0); // > CAPTURE-DR
        NEXUS_State(0); // > SHIFT-DR

        NEXUS_ShiftR(bitSz, outptr);
        outptr += quantSz;

        NEXUS_State(1); // > UPDATE-DR
    }

    // Zero the next location. Only needed if you read an uneven amount of bytes.
    // Since return length is patched to count even boundaries the host could be stupid and read 2/4 bytes expecting the extra byte to be 0
    *outptr = 0;

    NEXUS_State(0); // > RUN-TEST/IDLE / repeat RUN-TEST/IDLE if count is 0 (Should never happen)

    return RET_OK;
}

static uint16_t NEXUS3_SendData(const uint8_t bitSz, uint16_t Count, const void *in)
{
    const uint8_t *inptr = in;
    NEUXS_RWCS_t RWCS;
    *(uint32_t *) &RWCS = 0;
    uint8_t data    = 0xA << 1 | 1;
    uint8_t quantSz = bitSz / 8;

    RWCS.RW  = 1;            // 1 = Write
    RWCS.AC  = 1;            // Start access
    RWCS.SZ  = quantSz >> 1; // 0 = 8, 1 = 16, 2 = 32, 3 = 64
    RWCS.CNT = Count;        // NoAccess
    NEXUS3_WriteCommand(7, &RWCS);   // Configure RWCS register
    NEXUS3_ReadCommand(7, &RWCS);   // Read back, check status bits
    if (RWCS.ERR) return RWCS.DV ? RET_RESTRICMEM : RET_GENERICERR;

    while (Count--)
    {
        NEXUS_State(1); // > SELECT-DR-SCAN
        NEXUS_State(0); // > CAPTURE-DR
        NEXUS_State(0); // > SHIFT-DR

        NEXUS_ShiftW(8, &data);

        NEXUS_State(1); // > UPDATE-DR
        NEXUS_State(1); // > SELECT-DR-SCAN
        NEXUS_State(0); // > CAPTURE-DR
        NEXUS_State(0); // > SHIFT-DR

        NEXUS_ShiftW(bitSz, inptr);
        inptr += quantSz;

        NEXUS_State(1); // > UPDATE-DR
    }

    NEXUS_State(0); // > RUN-TEST/IDLE / repeat RUN-TEST/IDLE if count is 0 (Should never happen)

    return 0;
}

static uint16_t NEXUS1_Engage()
{
    const uint8_t data = ACCESS_AUX_TAP_ONCE;
    OnCE_OCMD_t OCMD;
    *(uint16_t *) &OCMD = 0;
    // printf("Engaging NEXUS 1\n\r");

    NEXUS_ResetState();

    NEXUS_WriteIREG(5, &data);
    OCMD.RS = OnCE_CMD_ENABLE_ONCE;
    NEXUS1_WriteOCMD(OCMD);

    nexuscurrgen = 1;

    return RET_OK;
}

static uint16_t NEXUS3_Engage()
{
    const uint8_t data = ACCESS_AUX_TAP_ONCE;
    OnCE_OCMD_t OCMD;
    OnCE_DBCR0_t DBCR0;
    OnCE_OCR_t   OCR;

    *(uint16_t *) &OCMD = 0;
    *(uint32_t *) &DBCR0 = 0;
    *(uint32_t *) &OCR   = 0;
    *(uint16_t *) &OnCE_OSR   = 0;

    // printf("Engaging NEXUS 3\n\r");

    NEXUS_ResetState();

    // Enter OnCE / NEXUS 1
    NEXUS_WriteIREG(5, &data);
    OCMD.RS = OnCE_CMD_ENABLE_ONCE;
    NEXUS1_WriteOCMD(OCMD);

    // "The CPU should be placed into debug mode through the OCR[DR] control bit before setting EDM"
    /// Enable MCU debug
    OCR.WKUP  = 1; // Force wakeup output to one
    OCR.DR    = 1; // Force DEBUG
    OCR.FDB   = 1;
    OCR.DMDIS = 1; // Disable MMU in debug
    uint32_t tOCR;
    NEXUS1_ReadOCMDRegister(OnCE_CMD_OCR, 1, &tOCR);

    tOCR |= (*(uint32_t *) &OCR);
    NEXUS1_WriteOCMDRegister(OnCE_CMD_OCR, 1, &tOCR);

    /////////////////////////////////////////////////////////////////////////////
    ////////////////////////// Problem area. OnCE does NOT accept the other bits!
    /// Enable OnCE debug
    // First pass. It'll ignore all other bits than "EDM"
    DBCR0.EDM = 1; // External debug mode
    OCMD.RS = OnCE_CMD_DBCR0;
    NEXUS1_WriteOCMDRegister(OnCE_CMD_DBCR0, 1, &DBCR0);

    // Second pass. EDM has been set, poke around with the other settings
    DBCR0.RST = 2; // 2: p_resetout_b set by debug reset control. Allows external device to initiate processor reset
    DBCR0.FT  = 1; // Disable timebase when halted
    NEXUS1_WriteOCMDRegister(OnCE_CMD_DBCR0, 1, &DBCR0);

    // Instruct OnCE to switch to NEXUS 3 mode
    OCMD.RS = OnCE_CMD_NEXUS3ACC;
    NEXUS1_WriteOCMD(OCMD);

    nexuscurrgen = 3;

    return RET_OK;
}

// Ah.. the silence was nice. Let's restore the whine
#pragma GCC diagnostic pop

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Public functions

// Prepare..
void NEXUS_setup(const float TargetFreq, const uint8_t generation)
{
    nexusdelay  = TAP_calcDelay(16, 1, TargetFreq);
    nexusmaxgen = generation;
}

// ..Commence
void NEXUS_TargetReady(const uint16_t *in, uint16_t *out)
{
    out[0] = RET_NOTSUP;

    if (nexusmaxgen == 1)
        out[0] = NEXUS1_Engage();
    else if (nexusmaxgen == 3)
        out[0] = NEXUS3_Engage();
}

// [register][size]
void NEXUS_ReadRegister(const uint16_t *in, uint16_t *out)
{
    if (*in == NEXUS_CPUSCR && in[1] != 24) {
        out[0] = RET_MALFORMED;
        return;
    }
    else if (in[1] != 4 && *in != JTAG_IREG && *in != JTAG_DREG && *in != NEXUS_CPUSCR) {
        out[0] = RET_MALFORMED;
        return;
    }

    out[0] = RET_OK;
    out[1] = 2 + 2;

    switch (*in)
    {
        // Hackjobs. Length specifies bits and not bytes
        case JTAG_IREG:
            out[1] = 2 + (in[1]>>4) + ((in[1]&0xF) ? 1 : 0);
            NEXUS_ReadIREG(in[1], &out[2]);
            return;
        case JTAG_DREG:
            out[1] = 2 + (in[1]>>4) + ((in[1]&0xF) ? 1 : 0);
            NEXUS_ReadDREG(in[1], &out[2]);
            return;


        // Misc
        case NEXUS_JTAGID:
            out[0] = NEXUS1_ReadCPUID(&out[2]);
            break;


        // CPUSCR
        case NEXUS_CPUSCR:
            out[0] = NEXUS1_ReadCPUSCR(&out[2]);
            out[1] = 2 + (6 * 2);
            break;
        case NEXUS_CPUSCR_CTL:
            out[0] = NEXUS1_ReadCPUSCR_CTL(&out[2]);
            break;
        case NEXUS_CPUSCR_IR:
            out[0] = NEXUS1_ReadCPUSCR_IR(&out[2]);
            break;
        case NEXUS_CPUSCR_PC:
            out[0] = NEXUS1_ReadCPUSCR_PC(&out[2]);
            break;
        case NEXUS_CPUSCR_MSR:
            out[0] = NEXUS1_ReadCPUSCR_MSR(&out[2]);
            break;
        case NEXUS_CPUSCR_WBBRUpper:
            out[0] = NEXUS1_ReadCPUSCR_WBBRUpper(&out[2]);
            break;
        case NEXUS_CPUSCR_WBBRLower:
            out[0] = NEXUS1_ReadCPUSCR_WBBRLower(&out[2]);
            break;

        default:
            *out = RET_NOTSUP;
            break;
    }

    if (*out != RET_OK)
        out[1] = 2;
}

// [register][size][data]++
void NEXUS_WriteRegister(const uint16_t *in, uint16_t *out)
{
    if (*in == NEXUS_CPUSCR && in[1] != 24) {
        out[0] = RET_MALFORMED;
        return;
    }
    else if (in[1] != 4 && *in != JTAG_IREG && *in != JTAG_DREG && *in != NEXUS_CPUSCR) {
        out[0] = RET_MALFORMED;
        return;
    }

    out[0] = RET_OK;

    switch (*in)
    {
    // Hackjobs. Length specifies bits and not bytes
    case JTAG_IREG:
        NEXUS_WriteIREG(in[1], &in[2]);
        return;
    case JTAG_DREG:
        NEXUS_WriteDREG(in[1], &in[2]);
        return;


    // CPUSCR
    case NEXUS_CPUSCR:
        out[0] = NEXUS1_WriteCPUSCR(&in[2]);
        break;
    case NEXUS_CPUSCR_CTL:
        out[0] = NEXUS1_WriteCPUSCR_CTL(&in[2]);
        break;
    case NEXUS_CPUSCR_IR:
        out[0] = NEXUS1_WriteCPUSCR_IR(&in[2]);
        break;
    case NEXUS_CPUSCR_PC:
        out[0] = NEXUS1_WriteCPUSCR_PC(&in[2]);
        break;
    case NEXUS_CPUSCR_MSR:
        out[0] = NEXUS1_WriteCPUSCR_MSR(&in[2]);
        break;
    case NEXUS_CPUSCR_WBBRUpper:
        out[0] = NEXUS1_WriteCPUSCR_WBBRUpper(&in[2]);
        break;
    case NEXUS_CPUSCR_WBBRLower:
        out[0] = NEXUS1_WriteCPUSCR_WBBRLower(&in[2]);
        break;

    default:
        out[0] = RET_NOTSUP;
        break;
    }
}

// [addr][addr],[len][len]
void NEXUS_ReadMemory(const uint16_t *in, uint16_t *out)
{
    uint32_t noBytes = *(uint32_t *) &in[2];

    out[0] = RET_MALFORMED;

    if (!noBytes || noBytes > MAX_RWLEN)
        return;

    NEXUS3_WriteCommand(9, in); // Configure RWA (Address)

    // 32-bit reads
    if (!(noBytes&3))
        out[0] = NEXUS3_FetchData(32, noBytes/4, &out[2]);
    // 8-bit reads
    else if (noBytes&1)
        out[0] = NEXUS3_FetchData( 8, noBytes  , &out[2]);
    // 16-bit reads.
    else
        out[0] = NEXUS3_FetchData(16, noBytes/2, &out[2]);

    noBytes += (noBytes&1);
    out[1]  += out[0] ? 0 : (noBytes / 2);
}

// [addr][addr],[len][len] [data]++
void NEXUS_WriteMemory(const uint16_t *in, uint16_t *out)
{
    uint32_t noBytes = *(uint32_t *) &in[2];

    out[0] = RET_MALFORMED;
    out[1] = 2;

    if (!noBytes || noBytes > MAX_RWLEN)
        return;

    NEXUS3_WriteCommand(9, in); // Configure RWA (Address)

    /// 32-bit writes
    if (!(noBytes&3))
        out[0] = NEXUS3_SendData(32, noBytes/4, &in[4]);
    /// 8-bit writes
    else if (noBytes&1)
        out[0] = NEXUS3_SendData( 8, noBytes  , &in[4]);
    /// 16-bit writes.
    else
        out[0] = NEXUS3_SendData(16, noBytes/2, &in[4]);
}

// TODO: Expand this thing! Read CTL etc...
static uint16_t NEXUS_HowMuchFailed()
{
    uint16_t fail = RET_OK;

    // Only print stuff that is out of the ordinary
    if (!OnCE_OSR.DEBUG) {
        fail = RET_UNKERROR;
        // printf("DEBUG  : %u\n\r", OnCE_OSR.DEBUG);
    }if (OnCE_OSR.STOP) {
        fail = RET_UNKERROR;
        // printf("STOP   : %u\n\r", OnCE_OSR.STOP);
    }if (OnCE_OSR.HALT) {
        fail = RET_UNKERROR;
        // printf("HALT   : %u\n\r", OnCE_OSR.HALT);
    }if (OnCE_OSR.RESET) {
        fail = RET_UNKERROR;
        // printf("RESET  : %u\n\r", OnCE_OSR.RESET);
    }if (OnCE_OSR.CHKSTOP) {
        fail = RET_UNKERROR;
        // printf("CHKSTOP: %u\n\r", OnCE_OSR.CHKSTOP);
    }if (OnCE_OSR.ERR) {
        fail = RET_GENERICERR;
        // printf("ERR    : %u\n\r", OnCE_OSR.ERR);
    }if (!OnCE_OSR.MCLK) {
        fail = RET_UNKERROR;
        // printf("MCLK   : %u\n\r", OnCE_OSR.MCLK);
    }

    return fail;
}

// Helper function
static void NEXUS_sStep(uint32_t Ins, uint32_t PC)
{
    OnCE_OCMD_t  OCMD;

    OCMD.RS = OnCE_CMD_CPUSCR;
    OCMD.EX = 0; // Do not exit debug
    OCMD.GO = 1; // Execute instruction
    OCMD.RW = 0; // Write data

    // Read back old CPUSCR since we must write all of them in one go
    NEXUS1_ReadCPUSCR(&OnCE_CPUSCR);
    // printf("\n\rBefore:\n\r");
    // printf("CTL    : %04X\n\r",(OnCE_CPUSCR.CTL)>>16);
    // printf("MSR    : %08X\n\r",OnCE_CPUSCR.MSR);
    // printf("Ins    : %08X\n\r",OnCE_CPUSCR.IR);
    // printf("PC     : %08X\n\r",OnCE_CPUSCR.PC);
    // printf("WBBRLow: %08X\n\r",OnCE_CPUSCR.WBBRLower);
    // printf("WBBRUp : %08X\n\r",OnCE_CPUSCR.WBBRUpper);

    // Update relevant parameters
    OnCE_CPUSCR.PC  = PC;
    OnCE_CPUSCR.IR  = Ins;

    // Workaround if anything..
    OnCE_CPUSCR.CTL = 0;

    // Send it!
    NEXUS1_WriteOCMD(OCMD);
    NEXUS_WriteDREG(32*6, &OnCE_CPUSCR);
}

static uint16_t NEXUS_ExecIns(const uint32_t instruction, const uint32_t PC)
{
    NEXUS_sStep(instruction, PC);

    // Read OCMD with a dummy command
    uint32_t tmp = 0;
    NEXUS1_ReadOCMDRegister(OnCE_CMD_NOREGSEL, 1, &tmp);

    return NEXUS_HowMuchFailed();
}

static uint16_t NEXUS_ExecIns_wRecData(const uint32_t instruction, const uint32_t PC, const uint16_t noBytes, void *out)
{
    uint32_t *ptr = out;
    uint16_t retval;

    NEXUS_sStep(instruction, PC);

    retval = NEXUS1_ReadCPUSCR(&OnCE_CPUSCR);
    if (retval != RET_OK) return retval;

    if (noBytes == 4)
        ptr[0] = OnCE_CPUSCR.WBBRLower;
    else if (noBytes == 8) {
        ptr[0] = OnCE_CPUSCR.WBBRLower;
        ptr[1] = OnCE_CPUSCR.WBBRUpper;
    }
    else
        return RET_NOTSUP;

    if ((OnCE_CPUSCR.CTL))
    {
        // printf("CTL: %08X\n\r",(OnCE_CPUSCR.CTL));
    }

    // It's a couple of layers down but NEXUS1_ReadCPUSCR will trigger an update of OSR
    retval = NEXUS_HowMuchFailed();
    if (retval != RET_OK)
    {
        // printf("NEXUS_ExecIns_wRecData(): Ins %08X Fail @ %08X\n\r", instruction, PC);
    }

    return retval;
}


// This one has another payload format than the other ones!
// In particular, you also have to pass along a program counter value to use


// [ 0 ] [[4][[INShi][INSlo]]] [[4][[PChi][PClo]]]                       : Just execute this instruction, no returned data.
// [ 2 ] [[4][[INShi][INSlo]]] [[data size out 4/8]] [[4][[PChi][PClo]]] : Execute instruction and read back data.
void NEXUS_ExecuteIns(const uint16_t *in, uint16_t *out)
{
    uint32_t Ins    = *(uint32_t *) &in[2];
    uint16_t Status = RET_OK;
    uint16_t outLen = 0;

    // Can only execute 32-bit instructions
    if (in[1] != 4) {
        out[0] = RET_MALFORMED;
        return;
    }

    // Only execute instruction
    if (*in == 0) {
        // Only 32-bit PC is supported
        if (in[4] != 4) {
            out[0] = RET_MALFORMED; return;
        }

        uint32_t PC = *(uint32_t *) &in[5];
        // printf("NEXUS_ExecuteIns(0).PC: %08X\n\r", PC);
        Status = NEXUS_ExecIns(Ins, PC);
    }

    // Execute and read data from target
    else if (*in == 2) {
        uint32_t PC      = *(uint32_t *) &in[6];
        uint16_t noBytes = in[4];
        // printf("NEXUS_ExecuteIns(2).PC: %08X\n\r", PC);

        // Can only read 4 or 8 bytes of data
        // Only 32-bit PC is supported
        if ((noBytes != 4 && noBytes != 8) || in[5] != 4) {
            out[0] = RET_MALFORMED; return;
        }

        Status = NEXUS_ExecIns_wRecData(Ins, PC, noBytes, &out[2]);
        outLen = noBytes / 2;
    }
    else
        out[0] = RET_NOTSUP;

    out[1] += (Status == RET_OK) ? outLen : 0;
    out[0]  = Status;
}

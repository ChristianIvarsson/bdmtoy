#include "../TAP_shared.h"
#include "BDMn_private.h"

#define NEWBDM_RETRIES 5
#define twelvedelay __asm("nop\n nop\n nop\n ")

extern uint32_t BDMNEW_DMADumpAS_USB_type2 (const void *ptr, uint32_t Address, uint32_t Length);
extern uint32_t BDMNEW_DMAFillAS           (const void *ptr, uint32_t Length);

// Strict aliasing rules... Please call nine whine whine.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"

static uint16_t BDMNEW_ReadGPR (const uint32_t Reg, void *out);
static uint16_t BDMNEW_WriteGPR(const uint32_t Reg, const void *in);
static uint32_t newbdm_delay = 0;

static void printRegSumamry()
{
    for (uint16_t i = 0; i < 32; i++) {
        uint32_t tmpr = 0;
        BDMNEW_ReadGPR(i, &tmpr);
        printf("%08X ", tmpr);
        if ((i&3)==3) printf(" (%2u - %2u)\n\r", i-3, i);
    }
}

// I wondered why kSec contained garbage while pointK was correct...
#pragma GCC push_options
#pragma GCC optimize ("O0")

void PrintTimeTaken(const uint32_t uSecs, const uint32_t noK)
{
    // "Floating point".. err.. or something
    uint32_t kSec = (uint32_t)((uint64_t)((noK * 10000000000) / (uSecs))/10),pointK = kSec;
    while (pointK > 9999) pointK -= 10000;
    while (pointK >  999) pointK -=  1000;
    kSec -= pointK;
    kSec /= 1000;

    // Sorry about the warning. Can't do much..
    printf("%u K took %u uSecs (%u mS) (%u.%03u KB/s)\n", (uint16_t) noK, uSecs, (uint16_t)((uint32_t)uSecs/1000), (uint16_t)kSec, (uint16_t)pointK);
}
#pragma GCC pop_options

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Lowest layer

// Clock out header and in status bits. Slow and safe
static inline uint32_t BDMNEW_ShiftHeader(const uint32_t Header)
{
    // Wait for target to enter "ready" state
    while ( TDO_RDs ) ;

    TDI_HI;
    CLK_HI;
    TAP_PreciseDelay( newbdm_delay );

    CLK_LO;
    TAP_PreciseDelay( newbdm_delay );

    ////////
    // Bit 0
    uint32_t Retval = TDO_RD;
    ( Header&1 ) ? TDI_HI : TDI_LO;

    CLK_HI;
    TAP_PreciseDelay( newbdm_delay );

    CLK_LO;
    TAP_PreciseDelay( newbdm_delay );

    ////////
    // Bit 1
    Retval |= TDO_RD << 1;
    ( Header&2 ) ? TDI_HI : TDI_LO;

    CLK_HI;
    TAP_PreciseDelay( newbdm_delay );

    CLK_LO;
    TAP_PreciseDelay( newbdm_delay );

    return Retval;
}

// 0x00
static inline uint32_t BDMNEW_ShiftHeaderFast_CORECMD()
{
    // Wait for target to enter "ready" state
    while ( TDO_RDs ) ;
    TDI_HI;

    CLK_HI;
    CLK_LO;
    twelvedelay;

    ////////
    // Bit 0
    uint32_t Retval = TDO_RD;
    TDI_LO;
    CLK_HI;
    CLK_LO;
    twelvedelay;

    ////////
    // Bit 1
    Retval |= TDO_RD  << 1;
    CLK_HI;
    CLK_LO;

    return Retval;
}

// 0x02
static inline void BDMNEW_ShiftHeaderFast_Download()
{
    while ( TDO_RDs ) ;
    TDI_HI;

    CLK_HI;
    CLK_LO;
    TDI_LO;
    twelvedelay;

    ////////
    // Bit 0
    CLK_HI;
    CLK_LO;
    TDI_HI;
    twelvedelay;

    ////////
    // Bit 1
    CLK_HI;
    CLK_LO;

}

// 0x03
static inline void BDMNEW_ShiftHeaderFast_Debug()
{
    while ( TDO_RDs ) ;
    TDI_HI;

    CLK_HI;
    CLK_LO;
    twelvedelay;

    CLK_HI;
    CLK_LO;
    twelvedelay;

    CLK_HI;
    CLK_LO;
}

static uint32_t BDMNEW_Shift(const uint32_t Header, const uint8_t noBits, uint32_t in, void *out)
{
    uint32_t  res  = 0, i, runtBits = 7;
    uint32_t  mask = 1 << (noBits - 1);
    uint32_t *out_ = out;

    uint32_t Status = BDMNEW_ShiftHeader(Header);

    // Catch previous mistakes, if any.
    // Reason for this is that the whole chain will desync if you try to send seven bits but the target expects you to retrieve thirty-two
    if (Status == NEWBDM_STAT_DATA)
        runtBits = 32;

    // Send and retrieve our bits
    for (i = 0; i < noBits; i++)
    {
        res <<= 1;
        res |= TDO_RD;

        ( in & mask ) ? TDI_HI : TDI_LO;
        in <<= 1;

        CLK_HI;
        TAP_PreciseDelay( newbdm_delay );

        CLK_LO;
        TAP_PreciseDelay( newbdm_delay );
    }

    // This is just here while debugging..
    if (i < runtBits)
    {
        for (uint32_t e = 0; e < 5; e++)
            printf("BDMNEW_Shift(): Target has more bits!\n\r");
    }

    TDI_LO;

    // If runtBits is higher than noBits; Continue since some _DUMBASS_ sent a read command and forgot to fetch the result..
    for ( ; i < runtBits; i++)
    {
        res <<= 1;
        res |= TDO_RD;

        CLK_HI;
        TAP_PreciseDelay( newbdm_delay );

        CLK_LO;
        TAP_PreciseDelay( newbdm_delay );
    }

    if (out_)
        *out_ = res;

    return Status;
}

// Execute instruction without returned data
static uint16_t BDMNEW_ExecIns(const uint32_t instruction)
{
    uint32_t Status, Res, Tries = NEWBDM_RETRIES;

Retry:
    BDMNEW_Shift(NEWBDM_CORECMD, 32, instruction, 0);
    Status = BDMNEW_Shift(NEWBDM_DEBUGCMD, 7, NEWBDM_CMD_NOP, &Res);

    // F...
    if (Status == NEWBDM_STAT_SEQERR ||
        Status == NEWBDM_STAT_CPUINT)
    {
        if (Tries--)
            goto Retry;
        return BDMNEW_TranslateFault(Status);
    }

    return RET_OK;
}

// Execute instruction with sent data
static uint16_t BDMNEW_ExecIns_wSentData(const uint32_t instruction, const void *in)
{
    uint32_t Status, Res, Tries = NEWBDM_RETRIES;

Retry:
    BDMNEW_Shift(NEWBDM_CORECMD, 32, instruction, 0);
    Status = BDMNEW_Shift(NEWBDM_DATACMD, 32, *(uint32_t *)in, &Res);

    // Check if instruction went through
    if (Status == NEWBDM_STAT_SEQERR ||
        Status == NEWBDM_STAT_CPUINT)
    {
        if (Tries--)
            goto Retry;
        return BDMNEW_TranslateFault(Status);
    }

    // Send dummy to check result of data
    Status = BDMNEW_Shift(NEWBDM_DEBUGCMD, 7, NEWBDM_CMD_NOP, &Res);

    if (Status == NEWBDM_STAT_SEQERR ||
        Status == NEWBDM_STAT_CPUINT)
    {
        if (Tries--)
            goto Retry;

        return BDMNEW_TranslateFault(Status);
    }

    return RET_OK;
}

// WARNING: SPI has to be configured for Tx ONLY before using this or future transfers _WILL_ fail!
static inline void BDMNEW_ExecIns_wSentDataFast(const uint32_t instruction, const void *in)
{
    //////////////
    // Instruction
    BDMNEW_ShiftHeaderFast_CORECMD();

    // Force compiler not to cheat
    volatile uint32_t tmp2 = instruction << 16 | (instruction >> 16);

    DMA1_Channel5->CMAR  = (uint32_t)&tmp2;
    DMA1_Channel5->CNDTR = 2;

    GPIOB->CRH          |= 0x80800000;
    DMA1_Channel5->CCR  |= DMA_CCR1_EN;

    while(DMA1_Channel5->CNDTR)        ;
    while(SPI2->SR & SPI_I2S_FLAG_BSY) ;

    DMA1_Channel5->CCR &= ~DMA_CCR1_EN;
    GPIOB->CRH         &= ~0x80800000;

    //////////////
    // Data
    BDMNEW_ShiftHeaderFast_Download();

    tmp2 = (*(uint32_t *) in) << 16 | ((*(uint32_t *) in) >> 16);

    DMA1_Channel5->CMAR  = (uint32_t)&tmp2;
    DMA1_Channel5->CNDTR = 2;

    GPIOB->CRH          |= 0x80800000;
    DMA1_Channel5->CCR  |= DMA_CCR1_EN;

    while(DMA1_Channel5->CNDTR)        ;
    while(SPI2->SR & SPI_I2S_FLAG_BSY) ;

    DMA1_Channel5->CCR &= ~DMA_CCR1_EN;
    GPIOB->CRH         &= ~0x80800000;
}

// Execute instruction with returned data
static uint16_t BDMNEW_ExecIns_wRecData(const uint32_t instruction, void *out)
{
    uint32_t Status, Tries = NEWBDM_RETRIES;

Retry:
    BDMNEW_Shift(NEWBDM_CORECMD, 32, instruction, 0);
    Status = BDMNEW_Shift(NEWBDM_CORECMD, 32, PPC_be_NOP, out);

    // F...
    if (Status != NEWBDM_STAT_DATA)
    {
        if (Tries--)
            goto Retry;
        return BDMNEW_TranslateFault(Status);
    }

    return RET_OK;
}

// Start download mode
static void BDMNEW_StartDownload(const uint32_t Startfrom)
{
    uint32_t tmp = Startfrom - 4;
    BDMNEW_ExecIns_wSentDataFast(PPC_be_MFSPR_DPDR(30), &tmp);

    // If this command fail, we'll simply catch it while trying to upload. Ignore status for now
    BDMNEW_ShiftHeaderFast_Debug();
    register uint32_t Command = NEWBDM_CMD_STARTDL;
    for (uint_fast8_t i = 0; i < 7; i++)
    {
        ( Command & 0x40 ) ? TDI_HI : TDI_LO;
        Command <<= 1;
        CLK_HI;
        CLK_LO;
    }
}

// Stop download mode
static void BDMNEW_StopDownload()
{
    // Send stop
    BDMNEW_ShiftHeaderFast_Debug();

    register uint32_t Command = NEWBDM_CMD_STOPTDL;

    for (uint_fast8_t i = 0; i < 7; i++)
    {
        ( Command & 0x40 ) ? TDI_HI : TDI_LO;
        Command <<= 1;
        CLK_HI;
        CLK_LO;
    }


    // Graceful shutdown
    BDMNEW_ShiftHeaderFast_Download();

    // Force compiler not to cheat
    volatile uint32_t tmp2 = 0;

    DMA1_Channel5->CMAR  = (uint32_t)&tmp2;
    DMA1_Channel5->CNDTR = 2;

    GPIOB->CRH          |= 0x80800000;
    DMA1_Channel5->CCR  |= DMA_CCR1_EN;

    while(DMA1_Channel5->CNDTR)        ;
    while(SPI2->SR & SPI_I2S_FLAG_BSY) ;

    DMA1_Channel5->CCR &= ~DMA_CCR1_EN;
    GPIOB->CRH         &= ~0x80800000;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// High level

static uint16_t BDMNEW_TranslateFault(uint32_t status)
{
    switch (status)
    {
        // Returned data, which was not expected
        case NEWBDM_STAT_DATA:
            return 0xFFFF;

        // Err.. You forgot to turn off an interrupt or two
        case NEWBDM_STAT_CPUINT:
            return 0xFFFF;

        // You're doing something in the wrong order
        case NEWBDM_STAT_SEQERR:
            return 0xFFFF;

        // Returned a null instead of data
        case NEWBDM_STAT_NULL:
            return 0xFFFF;
    }

    return 0xFFFF;
}

static uint16_t BDMNEW_ReadGPR(const uint32_t Reg, void *out) {
    return BDMNEW_ExecIns_wRecData(PPC_be_MTSPR_DPDR(Reg), out);
}

static uint16_t BDMNEW_WriteGPR(const uint32_t Reg, const void *in) {
    return BDMNEW_ExecIns_wSentData(PPC_be_MFSPR_DPDR(Reg), in);
}

static uint16_t BDMNEW_WriteSPR(const uint32_t Reg, const void *in)
{
    uint32_t r0 = 0;
    uint16_t retval;

    // Backup r0
    retval = BDMNEW_ReadGPR(0, &r0);
    if (retval != RET_OK) {
        printf("BDMNEW_ReadSPR(): Failed to read r0\n\r");
        return retval;
    }

    retval = BDMNEW_WriteGPR(0, in);
    if (retval != RET_OK) {
        printf("BDMNEW_ReadSPR(): Failed to write r0 (1)\n\r");
        return retval;
    }

    // Move from r0 to selected spr
    retval = BDMNEW_ExecIns( PPC_be_MTSPR_R0(Reg) );
    if (retval != RET_OK) {
        printf("BDMNEW_ReadSPR(): Failed to exec\n\r");
        return retval;
    }

    // Restore r0
    retval = BDMNEW_WriteGPR(0, &r0);
    if (retval != RET_OK)
        printf("BDMNEW_ReadSPR(): Failed to restore r0 (2)\n\r");

    return retval;
}

static uint16_t BDMNEW_ReadSPR(const uint32_t Reg, void *out)
{
    uint32_t r0 = 0;
    uint16_t retval;

    // Backup r0
    retval = BDMNEW_ReadGPR(0, &r0);
    if (retval != RET_OK) {
        printf("BDMNEW_ReadSPR(): Failed to read r0 (1)\n\r");
        return retval;
    }

    // Move from selected spr to r0
    retval = BDMNEW_ExecIns( PPC_be_MFSPR_R0(Reg) );
    if (retval != RET_OK) {
        printf("BDMNEW_ReadSPR(): Failed to exec\n\r");
        return retval;
    }

    // Read r0
    retval = BDMNEW_ReadGPR(0, out);
    if (retval != RET_OK) {
        printf("BDMNEW_ReadSPR(): Failed to read r0 (2)\n\r");
        return retval;
    }

    // Restore r0
    retval = BDMNEW_WriteGPR(0, &r0);
    if (retval != RET_OK)
        printf("BDMNEW_ReadSPR(): Failed to restore r0\n\r");

    return retval;
}

/*
static uint16_t BDMNEW_ReadMSR(void *out)
{
    uint32_t r0 = 0;
    uint16_t retval;

    // Backup r0
    retval = BDMNEW_ReadGPR(0, &r0);
    if (retval != RET_OK) {
        printf("BDMNEW_ReadSPR(): Failed to read r0 (1)\n\r");
        return retval;
    }

    // Move from msr to r0
    retval = BDMNEW_ExecIns( PPC_be_MFMSR_R0 );
    if (retval != RET_OK) {
        printf("BDMNEW_ReadSPR(): Failed to exec\n\r");
        return retval;
    }

    // Read r0
    retval = BDMNEW_ReadGPR(0, out);
    if (retval != RET_OK) {
        printf("BDMNEW_ReadSPR(): Failed to read r0 (2)\n\r");
        return retval;
    }

    // Restore r0
    retval = BDMNEW_WriteGPR(0, &r0);
    if (retval != RET_OK)
        printf("BDMNEW_ReadSPR(): Failed to restore r0\n\r");

    return retval;
}
*/

// Address must be aligned and length must be in multiples of four. Speed has its drawbacks
static uint16_t BDMNEW_FillMem(const uint32_t Address, const uint32_t Length, const void* in)
{
    SPI2->CR1 |= SPI_Direction_1Line_Tx;
    SPI2->CR2 &= ~SPI_I2S_DMAReq_Rx;

    BDMNEW_StartDownload(Address);

    uint16_t retval = BDMNEW_DMAFillAS(in, Length);

    BDMNEW_StopDownload();

    SPI2->CR2 |= SPI_I2S_DMAReq_Rx;
    SPI2->CR1 &= ~SPI_Direction_1Line_Tx;

    return retval;
}

static uint16_t BDMNEW_WriteMem(const uint32_t Address, const uint32_t Size, const void *in)
{
    uint32_t tmp = Address, r0 = 0, r1 = 0;
    uint16_t retval;

    // Backup
    retval = BDMNEW_ReadGPR(0, &r0);
    if (retval != RET_OK) return retval;

    retval = BDMNEW_ReadGPR(1, &r1);
    if (retval != RET_OK) return retval;

    // Store Address and data
    retval = BDMNEW_WriteGPR(1, &tmp);
    if (retval != RET_OK) goto Restore;

    retval = BDMNEW_WriteGPR(0, in);
    if (retval != RET_OK) goto Restore;

    if      (Size == 1) retval = BDMNEW_ExecIns(PPC_be_STB_r1);
    else if (Size == 2) retval = BDMNEW_ExecIns(PPC_be_STH_r1);
    else if (Size == 4) retval = BDMNEW_ExecIns(PPC_be_STW_r1);
    else                retval = RET_MALFORMED;

Restore:
    BDMNEW_WriteGPR(0, &r0);
    BDMNEW_WriteGPR(1, &r1);
    return retval;
}

static uint16_t BDMNEW_ReadMem(const uint32_t Address, const uint32_t Size, void *out)
{
    uint32_t tmp = Address, r0 = 0, r1 = 0;
    uint16_t retval;

    // Backup
    retval = BDMNEW_ReadGPR(0, &r0);
    if (retval != RET_OK) return retval;

    retval = BDMNEW_ReadGPR(1, &r1);
    if (retval != RET_OK) return retval;

    // Store Address
    retval = BDMNEW_WriteGPR(1, &tmp);
    if (retval != RET_OK)
        goto Restore;

    if (Size == 1)
        retval = BDMNEW_ExecIns(PPC_be_LBZ_r1);
    else if (Size == 2)
        retval = BDMNEW_ExecIns(PPC_be_LHZ_r1);
    else if (Size == 4)
        retval = BDMNEW_ExecIns(PPC_be_LWZ_r1);
    else
        retval = RET_MALFORMED;

    if (retval == RET_OK)
        retval = BDMNEW_ReadGPR(0, out);

Restore:
    BDMNEW_WriteGPR(0, &r0);
    BDMNEW_WriteGPR(1, &r1);
    return retval;
}

static uint16_t BDMNEW_InitPort_int()
{
    SetPinDir(P_Trst, 0);
    SetPinDir(P_CLK,0);
    // SetPinDir(P_RDY, 2);
    SetPinDir(P_TDI, 0);
    SetPinDir(P_TDO, 0);
    return RET_OK;
}

// Enable BDM:
// Clock high during reset
static uint16_t BDMNEW_TargetReset_int()
{
    BDMNEW_InitPort_int();

    SetPinDir(P_Trst, 1);

    RST_LO;
    sleep(100);

    SetPinDir(P_CLK , 1);
    SetPinDir(P_TDI , 1);

    // Set BDM mode
    TDI_LO;
    CLK_HI;

    SetPinDir(P_Trst, 3);

    // Give it enough time to sample pins and enter debug
    sleep(50);

    // Target alive. Set clock to the correct state
    CLK_LO;

    SetPinDir(P_CLK , 1);
    SetPinDir(P_TDI , 1);

    return RET_OK;
}

static inline uint32_t BDMNEW_PrealoadDump()
{
    uint32_t Status = BDMNEW_ShiftHeaderFast_CORECMD();

    GPIOB->CRH |= 0x80800000;

    SPI2->DR = 0x8401;
    while(!(SPI2->SR & SPI_I2S_FLAG_RXNE)) ;
    uint32_t tmp = SPI2->DR;

    SPI2->DR = 0x0004;
    while(!(SPI2->SR & SPI_I2S_FLAG_RXNE)) ;
    tmp = SPI2->DR;

    // Stupid warning. That register must be read..
    tmp = tmp;

    GPIOB->CRH &= ~0x80800000;

    return Status;
}

static uint16_t BDMNEW_FreezeStatus_int(uint16_t *out)
{
    uint32_t Res = 0;

    // Sacrificial command
    BDMNEW_Shift(NEWBDM_DEBUGCMD, 7, NEWBDM_CMD_NOP, 0);

    // Read actual status
    uint32_t Status = BDMNEW_Shift(NEWBDM_DEBUGCMD, 7, NEWBDM_CMD_NOP, &Res);

    *out = (Res&0x40) ? RET_TARGETSTOPPED : RET_TARGETRUNNING;

    // printf("\n\rFREEZE: %u\n\r", (uint8_t)(Res&0x40)>>6);
    // printf("DLMODE: %u\n\r\n\r", (uint8_t)(Res&0x20)>>5);
    // printf("Debug bits: %08X (%x)\n\r", Res, (uint16_t)Status);

    // Print out some crap if the target has stopped and all is OK
/*
    if ( (Res&0x40) && Status == 3)
    {
        // Read SRR1 16:31 (The half that is copied from MSR)
        uint32_t srr1    = 0;
        uint32_t Status2 = BDMNEW_ReadSPR(27, &srr1);
        if (Status2 != RET_OK) return Status;
        for (uint8_t i = 16; i < 32; i++) {
            if (srr1 & BIT_REVERSE(32,i))
                printf("SRR1 bit %u set\n\r", i);
        }
        if (srr1 & 0xFFFF) printf("\n\r");

        // Read ECR
        srr1    = 0;
        Status2 = BDMNEW_ReadSPR(148, &srr1);
        if (Status2 != RET_OK)  return Status;
        for (uint8_t i = 0; i < 32; i++) {
            if (srr1 & BIT_REVERSE(32,i))
                printf("ECR bit %2u set\n\r", i);
        }
        if (srr1) printf("\n\r");

        // Read DER
        srr1    = 0;
        Status2 = BDMNEW_ReadSPR(149, &srr1);
        if (Status2 != RET_OK) return Status;
        for (uint8_t i = 0; i < 32; i++) {
            if (srr1 & BIT_REVERSE(32,i))
                printf("DER bit %2u set\n\r", i);
        }
        if (srr1) printf("\n\r");

        // Read MSR
        srr1    = 0;
        Status2 = BDMNEW_ReadMSR(&srr1);
        if (Status2 != RET_OK) return Status;
        for (uint8_t i = 0; i < 32; i++) {
            if (srr1 & BIT_REVERSE(32,i))
                printf("MSR bit %2u set\n\r", i);
        }
    }
*/
    return (Status == 3) ? RET_OK : BDMNEW_TranslateFault(Status);
}

// You must configure ssr1 before calling this
static uint16_t BDMNEW_RunMode()
{
    uint32_t tmp = 0x0C000000;

    // Flush cache
    uint16_t Status = BDMNEW_WriteSPR(560, &tmp);
    if (Status != RET_OK) return Status;

    // Clear pending interrupts
    Status = BDMNEW_ReadSPR(148, &tmp);
    if (Status != RET_OK) return Status;

    BDMNEW_Shift(NEWBDM_CORECMD,32,PPC_be_RFI,0);
    return RET_OK;
}

static uint16_t BDMNEW_TargetStart_int()
{
    uint32_t srr1 = 0, der = 0;

    // Read DER
    uint16_t Status = BDMNEW_ReadSPR(149, &der);
    if (Status != RET_OK) return Status;

    der |=  BIT_REVERSE(32,31); // DPIE development port breakpoint enable
    der &= ~BIT_REVERSE(32,14); // Negate TRE just in case
    der |=  BIT_REVERSE(32, 8); //
    der |=  BIT_REVERSE(32, 6); //

    Status = BDMNEW_WriteSPR(149, &der);
    if (Status != RET_OK) return Status;

    Status = BDMNEW_ReadSPR(27, &srr1);
    if (Status != RET_OK) return Status;

    srr1 &= ~(BIT_REVERSE(32,21) | BIT_REVERSE(32,22));
    srr1 &= ~ BIT_REVERSE(32,17);

    Status = BDMNEW_WriteSPR(27, &srr1);
    if (Status != RET_OK) return Status;

    return BDMNEW_RunMode();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Public functions

void BDMNEW_InitPort(const uint16_t *in, uint16_t *out)
{   out[0] = BDMNEW_InitPort_int(); }
void BDMNEW_TargetReset(const uint16_t *in, uint16_t *out)
{   out[0] = BDMNEW_TargetReset_int(); }
void BDMNEW_TargetStart(const uint16_t *in, uint16_t *out)
{   out[0] = BDMNEW_TargetStart_int();                   }
void BDMNEW_TargetStatus(const uint16_t *in, uint16_t *out)
{   out[0] = BDMNEW_FreezeStatus_int(&out[2]), out[1] = 3; }

void BDMNEW_setup(const float TargetFreq, const uint16_t prescaler)
{
    // Hardware accelerated by SPI
    spi_cfg_t cfg;
    cfg.prescaler = prescaler;
    cfg.polarity  = SPI_LOW;
    cfg.phase     = SPI_FIRST;
    cfg.order     = SPI_MSB;
    cfg.size      = SPI_WORD;
    InitSPI(&cfg);

    newbdm_delay = TAP_calcDelay(24, 0.5, TargetFreq);
}

// [register][size][data]++
void BDMNEW_WriteRegister(const uint16_t *in, uint16_t *out)
{
    // printf("Write reg: %04u, %08X\n\r", in[0], *(uint32_t*)&in[2]);
    if ( in[1] != 4 )
        out[0] = RET_UNALIGNED;
    // GPR
    else if (in[0] < 32)
        out[0] = BDMNEW_WriteGPR(in[0], &in[2]);
    else
        out[0] = BDMNEW_WriteSPR(in[0] - 32, &in[2]);
}

// [register][size]
void BDMNEW_ReadRegister(const uint16_t *in, uint16_t *out)
{
    if ( in[1] != 4 )
        out[0] = RET_NOTSUP;

    // GPR
    else if (in[0] < 32) {
        out[0] = BDMNEW_ReadGPR(in[0], &out[2]);
        out[1] = 4;
    }
    else {
        out[0] = BDMNEW_ReadSPR(in[0] - 32, &out[2]);
        out[1] = 4;
    }
}

// [addr][addr],[len][len], [data]++
void BDMNEW_WriteMemory(const uint16_t *in, uint16_t *out)
{
    uint32_t Address  = *(uint32_t *) &in[0];
    uint32_t Len      = *(uint32_t *) &in[2];
    uint16_t *dataptr =  (uint16_t *) &in[4];
    uint16_t retval   = RET_OK;

    while (Len && retval == RET_OK)
    {
        // dword
        if (Len > 3) {
            retval = BDMNEW_WriteMem(Address, 4, dataptr);
            Len     -= 4;
            Address += 4;
            dataptr += 2;
        }
        // word
        else if (Len > 1) {
            retval = BDMNEW_WriteMem(Address, 2, dataptr);
            Len     -= 2;
            Address += 2;
            dataptr += 1;
        }
        // byte
        else {
            retval = BDMNEW_WriteMem(Address, 1, dataptr);
            Len     -= 1;
            Address += 1;
            dataptr += 1;
        }
    }

    out[0] = retval;
}

// In: [addr][addr],[len][len]
void BDMNEW_ReadMemory(const uint16_t *in, uint16_t *out)
{
    uint16_t *dataptr =  (uint16_t *) &out[2];
    uint32_t Address  = *(uint32_t *) &in[0];
    uint32_t Len      = *(uint32_t *) &in[2];
    uint16_t retval   = RET_OK;
    uint32_t noRead   = 0;

    while (noRead < Len && retval == RET_OK)
    {
        *dataptr = 0;
        // dword
        if (Len > 3) {
            retval = BDMNEW_ReadMem(Address, 4, dataptr);
            noRead  += 4;
            Address += 4;
            dataptr += 2;
        }
        // word
        else if (Len > 1) {
            retval = BDMNEW_ReadMem(Address, 2, dataptr);
            noRead  += 2;
            Address += 2;
            dataptr += 1;
        }
        // byte
        else {
            retval = BDMNEW_ReadMem(Address, 1, dataptr);
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
void BDMNEW_FillMemory(const uint16_t *in, uint16_t *out)
{
    TAP_ReadCMD_t *cmd = (TAP_ReadCMD_t *) in;
    out[0] = BDMNEW_FillMem(cmd->Address, cmd->Length, &in[4]);
}

void BDMNEW_DumpMemory(const uint16_t *in, uint16_t *out)
{
    TAP_ReadCMD_t *cmd = (TAP_ReadCMD_t *) in;

    uint32_t tmp = cmd->Address-4;

    // Not the best solution..
    if (BDMNEW_WriteGPR(1, &tmp) || BDMNEW_PrealoadDump() != 3) {
        out[0] = RET_UNKERROR;
        return;
    }

    if (BDMNEW_DMADumpAS_USB_type2(out, cmd->Address, cmd->Length))
        out[0] = RET_UNKERROR;
}

// [ 0 ] [[4][[INShi][INSlo]]]              : Just execute this instruction, no returned data.
// [ 1 ] [[4][[INShi][INSlo]]] [sizeB][data]: Execute instruction and send data to target.
// [ 2 ] [[4][[INShi][INSlo]]] [sizeB]      : Execute instruction and read back data.
// [ 3 ] [[4][[INShi][INSlo]]] [sizeB][data]: Execute instruction with sent data, return received data.

void BDMNEW_ExecuteIns(const uint16_t *in, uint16_t *out)
{
    uint32_t Ins    = *(uint32_t *) &in[2];
    uint16_t Status = RET_OK;
    uint16_t outLen = 0;

    // Can only execute 32-bit instructions
    if (in[1] != 4) {
        out[0] = RET_MALFORMED; return;
    }

    // Only execute instruction
    if (*in == 0)
        Status = BDMNEW_ExecIns(Ins);

    // Execute and send data to target
    else if (*in == 1) {
        // Can only send 32-bit package
        if (in[4] != 4) {
            out[0] = RET_MALFORMED; return;
        }

        Status = BDMNEW_ExecIns_wSentData(Ins, &in[5]);
    }

    // Execute and read data from target
    else if (*in == 2) {
        // Can only receive 32-bit package
        if (in[4] != 4) {
            out[0] = RET_MALFORMED; return;
        }

        outLen = 2;
        Status = BDMNEW_ExecIns_wRecData(Ins, &out[2]);
    }

    // No need for type 3 yet
    else
        out[0] = RET_NOTSUP;

    out[1] += (Status == RET_OK) ? outLen : 0;
    out[0]  = Status;
}

// [addr][addr],[len][len]
void BDMNEW_AssistFlash(const uint16_t *in, uint16_t *out)
{
    TAP_AssistCMD_t *cmd = (TAP_AssistCMD_t *) in;
    uint32_t Last   = cmd->Address + cmd->Length;
    uint16_t retval;
    uint32_t regRead = 1;

    // Since we intend to thrash a bunch of buffers we better store the data of these pointers.
    uint32_t driverStart = cmd->DriverStart;
    uint32_t bufferStart = cmd->BufferStart;
    uint32_t bufferLen   = cmd->BufferLen;
    uint32_t flashStart  = cmd->Address;
/*
    printRegSumamry();
    printf("Flash Length : %08X\n\r", cmd->Length);
    printf("Driver start : %08X\n\r", driverStart);
    printf("Buffer start : %08X\n\r", bufferStart);
    printf("Buffer length: %08X\n\r", bufferLen);
    printf("Flash start  : %08X\n\r", flashStart);
*/
    while (flashStart < Last)
    {
        // printf("Flash start  : %08X\n\r", flashStart);
        uint16_t *ptr = TAP_RequestData(flashStart, bufferLen);

        // Got no data!
        if (!ptr) {
            TAP_UpdateStatus(RET_NOPTR, 0);
            return;
        }

        // while running..
        set_Timeout(60000);
        regRead = RET_TARGETRUNNING;
        do { retval = BDMNEW_FreezeStatus_int((uint16_t*)&regRead);
        } while ( retval == RET_OK && regRead == RET_TARGETRUNNING && !get_Timeout() );

        if (retval != RET_OK) {
            TAP_UpdateStatus(RET_RWERR, retval);
            return;
        }
        // Timeout
        else if (get_Timeout()) {
            TAP_UpdateStatus(RET_TIMEOUT, RET_TARGETRUNNING);
            return;
        }

        // Read status from r3
        retval = BDMNEW_ReadGPR(3, &regRead);
        if (retval != RET_OK) {
            TAP_UpdateStatus(RET_RWERR, retval);
            return;
        }

        // Driver says something went wrong..
        else if (regRead != 1) {
            printRegSumamry();
            TAP_UpdateStatus(RET_DRIVERFAIL, retval);
            return;
        }

        // Set PC
        retval = BDMNEW_WriteSPR(26, &driverStart);
        if (retval != RET_OK) {
            TAP_UpdateStatus(RET_RWERR, retval);
            return;
        }

        // Fill memory
        retval = BDMNEW_FillMem(bufferStart, bufferLen, ptr);
        if (retval != RET_OK) {
            TAP_UpdateStatus(RET_RWERR, retval);
            return;
        }

        // Start target
        retval = BDMNEW_TargetStart_int();
        if (retval != RET_OK) {
            TAP_UpdateStatus(RET_NOSTART, retval);
            return;
        }

        flashStart += bufferLen;
    }

    // printf("Loop done\n\r");
    // sleep(4);

    regRead = RET_TARGETRUNNING;
    // while running..
    set_Timeout(60000);
    do { retval = BDMNEW_FreezeStatus_int((uint16_t *) &regRead);
    } while (retval == RET_OK && regRead == RET_TARGETRUNNING && !get_Timeout());

    if (retval != RET_OK) {
        TAP_UpdateStatus(RET_RWERR, retval);
        return;
    }
    // Timeout
    else if (get_Timeout()) {
        TAP_UpdateStatus(RET_TIMEOUT, RET_TARGETRUNNING);
        return;
    }

    retval = BDMNEW_ReadGPR(3, &regRead);
    // Could not fetch status
    if (retval != RET_OK) {
        TAP_UpdateStatus(RET_RWERR, retval);
        return;
    }

    // Driver says something went wrong..
    else if (regRead != 1) {
        TAP_UpdateStatus(RET_DRIVERFAIL, retval);
        return;
    }

    // printf("Operation done\n\r");
    TAP_UpdateStatus(RET_OK, RET_OK);
}

void BDMNEW_ReleaseTarg(const uint16_t *in, uint16_t *out)
{
    BDMNEW_InitPort_int();

    // Do the opposite of Init
    SetPinDir(P_CLK , 1);
    SetPinDir(P_TDI , 1);
    TDI_HI;
    CLK_LO;

    SetPinDir(P_Trst, 1);
    RST_LO;
    sleep(100);

    SetPinDir(P_Trst, 3);
    sleep(100);

    TAP_InitPins();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Toys

/*
static uint16_t MPC562Init()
{
    // *F*ck you, watchdog!*
    uint16_t SYPCR = 0;
    uint16_t retval = BDMNEW_ReadMem(0x2FC006, 2, &SYPCR);
    if (retval != RET_OK)
        return retval;

    printf("SYPCR: %08x\n\r", SYPCR);
    SYPCR |= 1 << 3; // Set SWF
    SYPCR &= 0xFFFB; // Negate SWE

    retval = BDMNEW_WriteMem(0x2FC006, 2, &SYPCR);
    printf("SYPCR: %08x\n\r", SYPCR);

    // SYPCR = 0;
    // retval = BDMNEW_ReadMem(0x2FC286, 2, &SYPCR);
    // printf("PLPRCR: %08x\n\r", SYPCR);

    // SYPCR |= 0x80;
    // printf("PLPRCR: %08x\n\r", SYPCR);

    // retval = BDMNEW_WriteMem(0x2FC286, 2, &SYPCR);
    // sleep(100);

    // retval = BDMNEW_ReadMem(0x2FC286, 2, &SYPCR);
    // printf("PLPRCR: %08x\n\r", SYPCR);
    return retval;
}

// Ver:  0002   Rev:  0020
// Part: 3530   Mask: 0000
void MPCBDMTEST()
{

    // playdma();
    BDMNEW_TargetReset_int();

    BDMNEW_setup(56000000, 0);

    uint16_t tmeeep;
    BDMNEW_FreezeStatus_int(&tmeeep);
    return;

    if (MPC562Init() != RET_OK)
    {
        printf("Init fail!\n\r");
        return;
    }

    uint16_t data[2];
    *(uint32_t *) &data = 0;
    BDMNEW_ReadSPR(287,&data);
    printf("Ver:  %04X   Rev:  %04X\n\r", data[1], data[0]);

    *(uint32_t *) &data = 0;
    BDMNEW_ReadSPR(638,&data);
    printf("Part: %04X   Mask: %04X\n\r", data[1], data[0]);

    uint16_t tempdata[4];

    if (BDMNEW_ReadMem(0, 4, &tempdata)) {
        printf("test could not read\n\r");
    }
    if (BDMNEW_ReadMem(0, 4, &tempdata[2])) {
        printf("test could not read\n\r");
    }
    printf("\n\rFLASH[0][4]: %04X%04X %04X%04X\n\r",tempdata[1],tempdata[0],tempdata[3],tempdata[2]);

    for (uint32_t i = 0; i < 512; i++)
        receiveBuffer[i] = i+1;

    uint32_t Len = 2048*1024;
    uint32_t Address = 0x3F8000;

    printf("\n\rStart fill\n\r");
    uint32_t old = DWT->CYCCNT;
    while (Len)
    {
        BDMNEW_FillMem(Address,1024,&receiveBuffer);

        Len     -= 1024;
        Address += 1024;

        if (Address >= (0x3F8000+32768))
            Address =0x3F8000;
    }

    PrintTimeTaken((DWT->CYCCNT - old) / 48, 2048);

    printf("\n\rReadback\n\r");
    if (BDMNEW_ReadMem(0x3F8000, 4, &tempdata))
        printf("test could not read\n\r");
    if (BDMNEW_ReadMem(0x3F8004, 4, &tempdata[2]))
        printf("test could not read\n\r");

    printf("SRAM[0][4] : %04X%04X %04X%04X\n\r", tempdata[1], tempdata[0], tempdata[3], tempdata[2]);
}
*/

// Ah.. the silence was nice. Let's restore the whine
#pragma GCC diagnostic pop

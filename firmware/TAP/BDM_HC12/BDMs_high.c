/* sBDM high level code */
#include "../TAP_shared.h"
#include "BDMs_private.h"

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// "Private" to this interface/file

///////////////////////////////////
// Defines, external functions etc

// I know it's messy but it was easier to pass along delays and such this way!
typedef struct __attribute((packed)){
    volatile uint32_t payload;       // Data in and out. Remember to shift up data to be sent since we're MSB first!
    volatile uint32_t noBits;        // Number of bits to send or receive.

    // Delay parameters.
    volatile uint32_t delayKeepLow;  // Add this many loops to keep bkgnd low for FOUR target ticks.
    volatile uint32_t delayToHigh;   // Add this many loops to bring bkgnd high after NINE target ticks.
    volatile uint32_t delayKeepHigh; // Add this many loops to keep bkgnd high for THREE target ticks.
    volatile uint32_t delaySample;   // Add this many loops so that we sample bkgnd SIX target ticks after we released the pin.

    volatile uint32_t pinCfgMASK;
    volatile uint32_t *pinCfgPtr;
    volatile uint32_t *pinClrPtr;
    volatile uint32_t *pinSetPtr;
    volatile uint32_t *pinPtr;

    // You want to time EXACTLY how many host cycles something took? -Look no further..
    volatile uint32_t *dwtptr;
    volatile uint32_t benchTime;
} sbdm_td;

// Allocate our array of bdm parameters
sbdm_td sbdmpack;

extern void sBDMsendBits(sbdm_td *params);
extern void sBDMreceiveBits(sbdm_td *params);
extern void sBDMextraDelay(uint32_t ticks);

static uint32_t bc150Delay;
static uint32_t bc64Delay;
static uint32_t bc44Delay;
static uint32_t bc32Delay;

static void BDM_HCS12_InitPort_int()
{
    SetPinDir(P_Trst, 3); // RESET
    SetPinDir(P_CLK , 0); // BKGND
    CLK_HI;
    RST_HI;
}

static uint16_t BDM_HCS12_TargetReset_int()
{
    BDM_HCS12_InitPort_int();

    SetPinDir(P_Trst, 1);
    RST_LO;
    sleep(50);

    BDM_HCS12_InitPort_int();

    set_Timeout(500);
    while(!RST_RD && !get_Timeout())   ;
    // disable_Timeout();

    return RST_RD ? RET_OK : RET_NOSTART;
}

static uint16_t BDM_HCS12_TargetReady_int()
{
    BDM_HCS12_InitPort_int();

    SetPinDir(P_Trst, 1);
    RST_LO;
    sleep(10);

    SetPinDir(P_CLK , 1);
    CLK_LO;
    sleep(50);

    SetPinDir(P_Trst, 3);

    set_Timeout(500);
    while(!RST_RD && !get_Timeout())   ;
    // disable_Timeout();

    // Release bkgnd
    sleep(100);
    BDM_HCS12_InitPort_int();

    return RST_RD ? RET_OK : RET_NOSTART;
}

/////////////////////////////////////////////////////////////
/// Go, bdm, trace...

static uint8_t sendCommand(const uint8_t Command) {

    sbdmpack.payload = Command;
    sbdmpack.noBits = 8;
    sBDMsendBits(&sbdmpack);

    TAP_PreciseDelay(bc64Delay);
    return RET_OK;
}

/////////////////////////////////////////////////////////////
/// sBDM firmware must be active for these to function
static uint16_t readFirmware(const uint8_t Command) {

    sbdmpack.payload = Command;
    sbdmpack.noBits = 8;
    sBDMsendBits(&sbdmpack);

    TAP_PreciseDelay(bc44Delay);

    sbdmpack.noBits = 16;
    sBDMreceiveBits(&sbdmpack);

    return sbdmpack.payload;
}

static uint8_t writeFirmware(const uint8_t Command, const uint16_t Data) {

    sbdmpack.payload = Command;
    sbdmpack.noBits = 8;
    sBDMsendBits(&sbdmpack);

    sbdmpack.payload = Data;
    sbdmpack.noBits = 16;
    sBDMsendBits(&sbdmpack);

    TAP_PreciseDelay(bc32Delay);
    return RET_OK;
}

/////////////////////////////////////////////////////////////
/// BDM registers mapped onto the regular memory space
static uint8_t readBDByte(const uint16_t Address) {

    sbdmpack.payload = BDMHCS12_READ_BD_BYTE;
    sbdmpack.noBits = 8;
    sBDMsendBits(&sbdmpack);

    sbdmpack.payload = Address;
    sbdmpack.noBits = 16;
    sBDMsendBits(&sbdmpack);

    TAP_PreciseDelay(bc150Delay);

    sBDMreceiveBits(&sbdmpack);

    return ((Address&1) ? sbdmpack.payload : sbdmpack.payload >> 8);
}

static uint16_t readBDWord(const uint16_t Address) {

    sbdmpack.payload = BDMHCS12_READ_BD_WORD;
    sbdmpack.noBits = 8;
    sBDMsendBits(&sbdmpack);

    sbdmpack.payload = Address;
    sbdmpack.noBits = 16;
    sBDMsendBits(&sbdmpack);

    TAP_PreciseDelay(bc150Delay);

    sBDMreceiveBits(&sbdmpack);

    return sbdmpack.payload;
}

static uint8_t writeBDByte(const uint16_t Address, const uint8_t Data) {

    sbdmpack.payload = BDMHCS12_WRITE_BD_BYTE;
    sbdmpack.noBits = 8;
    sBDMsendBits(&sbdmpack);

    sbdmpack.noBits = 16;

    sbdmpack.payload = Address;
    sBDMsendBits(&sbdmpack);

    sbdmpack.payload = ((Address&1) ? Data : Data << 8);
    sBDMsendBits(&sbdmpack);

    TAP_PreciseDelay(bc150Delay);
    return 0;
}

static uint8_t writeBDWord(const uint16_t Address, const uint16_t Data) {

    sbdmpack.payload = BDMHCS12_WRITE_BD_WORD;
    sbdmpack.noBits = 8;
    sBDMsendBits(&sbdmpack);

    sbdmpack.noBits = 16;

    sbdmpack.payload = Address;
    sBDMsendBits(&sbdmpack);

    sbdmpack.payload = Data;
    sBDMsendBits(&sbdmpack);

    TAP_PreciseDelay(bc150Delay);
    return 0;
}

/////////////////////////////////////////////////////////////
/// Pretty self-explanatory functions
static uint8_t readByte(const uint16_t Address) {

    sbdmpack.payload = BDMHCS12_READ_BYTE;
    sbdmpack.noBits = 8;
    sBDMsendBits(&sbdmpack);

    sbdmpack.payload = Address;
    sbdmpack.noBits = 16;
    sBDMsendBits(&sbdmpack);

    TAP_PreciseDelay(bc150Delay);

    sBDMreceiveBits(&sbdmpack);

    return ((Address&1) ? sbdmpack.payload : sbdmpack.payload >> 8);
}

static uint16_t readWord(const uint16_t Address) {

    sbdmpack.payload = BDMHCS12_READ_WORD;
    sbdmpack.noBits = 8;
    sBDMsendBits(&sbdmpack);

    sbdmpack.payload = Address;
    sbdmpack.noBits = 16;
    sBDMsendBits(&sbdmpack);

    TAP_PreciseDelay(bc150Delay);

    sBDMreceiveBits(&sbdmpack);

    return sbdmpack.payload;
}

static uint8_t writeByte(const uint16_t Address, const uint8_t Data) {

    // printf("HCS_writeByte: %02X\n\r", Data);

    sbdmpack.payload = BDMHCS12_WRITE_BYTE;
    sbdmpack.noBits = 8;
    sBDMsendBits(&sbdmpack);

    sbdmpack.noBits = 16;

    sbdmpack.payload = Address;
    sBDMsendBits(&sbdmpack);

    sbdmpack.payload = ((Address&1) ? Data : Data << 8);
    sBDMsendBits(&sbdmpack);

    TAP_PreciseDelay(bc150Delay);
    return RET_OK;
}

static uint8_t writeWord(const uint16_t Address, const uint16_t Data) {

    sbdmpack.payload = BDMHCS12_WRITE_WORD;
    sbdmpack.noBits = 8;
    sBDMsendBits(&sbdmpack);

    sbdmpack.noBits = 16;

    sbdmpack.payload = Address;
    sBDMsendBits(&sbdmpack);

    sbdmpack.payload = Data;
    sBDMsendBits(&sbdmpack);

    TAP_PreciseDelay(bc150Delay);
    return RET_OK;
}

static void bdms_blastReceiver(uint32_t Start, const uint32_t End, uint16_t *buffer)
{
    uint16_t first = 1;
    // printf("Poor host...\n\r");

    // [tot len] [cmd][sts] [addr][addr] [Data..]
    buffer[1] = TAP_DO_DUMPMEM;
    buffer[2] = RET_OK;

    uint32_t BytesToCopy = 128;

    // Set X to desired start address
    writeFirmware(BDMHCS12_WRITE_X, Start);

    while (Start < End)
    {
        if (End-Start < BytesToCopy)
            BytesToCopy=End-Start;

        buffer[0] = (BytesToCopy / 2) + 5;
        *(uint32_t *) &buffer[3] = Start;

        uint16_t *ptr = (uint16_t *) &buffer[5];

        // Since we can only read the next address with the dump command, we have to do things slightly different when starting.
        if (first)
        {
            *ptr++ = readWord(Start);

            if (BytesToCopy > 2)
            {
                for (uint_fast16_t i = 1; i < BytesToCopy/2; i ++)
                    *ptr++ = readFirmware(BDMHCS12_READ_NEXT);
            }

            first = 0;
        }
        // Awyeah! We've started and can clock in data like there's no tomorrow.
        else
        {
            for (uint_fast16_t i = 0; i < BytesToCopy/2; i ++)
                *ptr++ = readFirmware(BDMHCS12_READ_NEXT);
        }


        Start += BytesToCopy;

        if (usb_sendData(buffer) != RET_OK)
        {
            // printf("Failed to send!\n\r");
            buffer[0] = RET_UNKERROR;
            return;
        }
    }
    buffer[0] = RET_OK;
    // printf("Done\n\r");
}

static uint16_t BDM_HCS12_stuffTarget(const uint32_t Start, uint32_t Len, const uint16_t *buffer)
{
    uint16_t retval = RET_OK;

    // Set X to desired start address
    writeFirmware(BDMHCS12_WRITE_X, Start);

    writeWord(Start, *buffer++);
    Len   -= 2;

    while (Len && retval == RET_OK)
    {
        writeFirmware(BDMHCS12_WRITE_NEXT, *buffer++);
        Len-=2;
    }

    return retval;
}

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Public functions

void BDMHCS12_setup(const float TargetFreq)
{
    sbdmpack.dwtptr     = (uint32_t *) &DWT->CYCCNT;
    sbdmpack.pinCfgMASK = ~(0xF << (5 * 4));
    sbdmpack.pinCfgPtr  = (uint32_t *) &GPIOB->CRH;
    sbdmpack.pinClrPtr  = (uint32_t *) &GPIOB->BRR;
    sbdmpack.pinSetPtr  = (uint32_t *) &GPIOB->BSRR;
    sbdmpack.pinPtr     = (uint32_t *) &GPIOB->IDR;

    sbdmpack.delayKeepLow  = TAP_calcDelay(16, 4, TargetFreq); // Keep low for FOUR cycles
    sbdmpack.delayToHigh   = TAP_calcDelay(19, 9, TargetFreq); // Hold data bit for NINE cycles
    sbdmpack.delayKeepHigh = TAP_calcDelay(16, 3, TargetFreq); // Keep high for THREE cycles
    sbdmpack.delaySample   = TAP_calcDelay(17, 6, TargetFreq); // Sample SIX cycles after pin was released

    bc150Delay = TAP_calcDelay(0, 150, TargetFreq);
    bc64Delay  = TAP_calcDelay(0,  64, TargetFreq);
    bc44Delay  = TAP_calcDelay(0,  44, TargetFreq);
    bc32Delay  = TAP_calcDelay(0,  32, TargetFreq);
}

// Init pins
void BDMHCS12_InitPort(const uint16_t *in, uint16_t *out)
{   BDM_HCS12_InitPort_int(); out[0] = RET_OK;              }
// Only reset
void BDMHCS12_TargetReset(const uint16_t *in, uint16_t *out)
{   out[0] =  BDM_HCS12_TargetReset_int();                  }
// Reset and enter background
void BDMHCS12_TargetReady(const uint16_t *in, uint16_t *out)
{   out[0] = BDM_HCS12_TargetReady_int();                   }

// [addr][addr],[len][len]
void BDMHCS12_DumpMemory(const uint16_t *in, uint16_t *out)
{
    TAP_ReadCMD_t *cmd = (TAP_ReadCMD_t *) in;
    uint32_t Address = cmd->Address;
    uint32_t Length  = cmd->Length;

    bdms_blastReceiver(Address, Address + Length, out);
}

// [addr][addr],[len][len] [data]++
void  BDMHCS12_WriteMemory(const uint16_t *in, uint16_t *out)
{
    uint32_t Address  = *(uint32_t *) &in[0];
    uint32_t Len      = *(uint32_t *) &in[2];
    uint16_t *dataptr =  (uint16_t *) &in[4];
    uint16_t retval   = RET_OK;

    // BDM goes wonkers if you write words to bdm locations.. Should perhaps flag a fault and abort if the host tries to write more than a byte?

    while (Len && retval == RET_OK)
    {
        // word
        if (Len > 1) {
            retval = (Address > 0xffff) ? writeBDWord(Address&0xffff, *dataptr++) : writeWord(Address, *dataptr++);
            Len     -= 2;
            Address += 2;
        }
        // byte
        else {
            retval = (Address > 0xffff) ? writeBDByte(Address&0xffff, *dataptr) : writeByte(Address, *dataptr);
            Len = 0;
        }
    }

    out[0] = retval;
}

// [addr][addr],[len][len]
void BDMHCS12_ReadMemory(const uint16_t *in, uint16_t *out)
{
    uint16_t *dataptr =  (uint16_t *) &out[2];
    uint32_t Address  = *(uint32_t *) &in[0];
    uint32_t Len      = *(uint32_t *) &in[2];
    uint16_t retval   = RET_OK;
    uint32_t noRead   = 0;

    // BDM goes wonkers if you read words from bdm locations.. Should perhaps flag a fault and abort if the host tries to read more than a byte?

    while (noRead < Len && retval == RET_OK)
    {
        // dword
        if (Len > 1) {
            *dataptr++ = (Address > 0xffff) ? readBDWord(Address&0xffff) : readWord(Address);
            noRead  += 2;
            Address += 2;
        }
        // byte
        else {
            *dataptr++ = (Address > 0xffff) ? readBDByte(Address&0xffff) : readByte(Address);
            noRead++;
        }
    }

    out[0] = retval;
    out[1] = 2 + (noRead>>1) + (noRead&1);
}

// [addr][addr],[len][len], [data..]
void BDMHCS12_FillMemory(const uint16_t *in, uint16_t *out)
{
    TAP_ReadCMD_t *cmd = (TAP_ReadCMD_t *) in;
    out[0] = BDM_HCS12_stuffTarget(cmd->Address, cmd->Length, (const uint16_t *) &in[4]);
}

// [register][size][data]++
void BDMHCS12_WriteRegister(const uint16_t *in, uint16_t *out)
{
    switch (*in)
    {
        case BDMHCS12_WRITE_NEXT:
        case BDMHCS12_WRITE_PC:
        case BDMHCS12_WRITE_D:
        case BDMHCS12_WRITE_X:
        case BDMHCS12_WRITE_Y:
        case BDMHCS12_WRITE_SP:
            if (in[1] != 2) {
                out[0] = RET_UNALIGNED;
                return;
            }
            writeFirmware(*in, in[2]);
            out[0] = RET_OK;
            out[1] = 2;
            return;
        default:
            out[0] = RET_NOTSUP;
            break;
    }
}

// [register][size]
void BDMHCS12_ReadRegister(const uint16_t *in, uint16_t *out)
{
    switch (*in)
    {
        case BDMHCS12_READ_NEXT:
        case BDMHCS12_READ_PC:
        case BDMHCS12_READ_D:
        case BDMHCS12_READ_X:
        case BDMHCS12_READ_Y:
        case BDMHCS12_READ_SP:
            if (in[1] != 2) {
                out[0] = RET_UNALIGNED;
                return;
            }
            out[2] = readFirmware(*in);
            out[0] = RET_OK;
            out[1] = 3;
            // The target only returns words and that is what the host gets
            return;
        default:
            out[0] = RET_NOTSUP;
            break;
    }
}

void BDMHCS12_TargetStart(const uint16_t *in, uint16_t *out)
{   out[0] = sendCommand(BDMHCS12_GO);                      }

void BDMHCS12_ReleaseTarg(const uint16_t *in, uint16_t *out)
{   out[0] = BDM_HCS12_TargetReset_int(); }

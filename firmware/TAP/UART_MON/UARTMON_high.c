#include "../TAP_shared.h"
#include "UARTMON_private.h"


/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Internal

// #define USART_Mode_Rx                        ((uint16_t)0x0004)
// #define USART_Mode_Tx                        ((uint16_t)0x0008)
static uint16_t UARTMON_putc(uint8_t data)
{
    volatile uint16_t rec = ~data;

    // while(!USART_GetFlagStatus(USART1, USART_FLAG_TXE))  ;

    // Fuck off, interrupts!
    // __asm volatile("cpsid i");

    // Clear old junk
    if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE))
        rec = USART1->DR&0xffff;

    // Send
    USART1->DR = data;

    while(!USART_GetFlagStatus(USART1, USART_FLAG_TXE))  ;

    // Enable interrupts
    // __asm volatile("cpsie i");

    // This is just the self-received byte..
    set_Timeout(500);
    while (!(USART_GetFlagStatus(USART1, USART_FLAG_RXNE)) && !get_Timeout())  ;
    if (!get_Timeout())
        rec = USART1->DR&0xffff;

    return (rec == data ? RET_OK : 0xffff);
}

static uint16_t UARTMON_sendByte(uint8_t data)
{
    uint16_t rec = ~data;

    if (UARTMON_putc(data) != RET_OK)
    {
        return 0xffff;
    }

    // Ack byte sent by the mpu
    set_Timeout(500);
    while (!(USART_GetFlagStatus(USART1, USART_FLAG_RXNE)) && !get_Timeout())  ;
    if (!get_Timeout())
    {
        rec = USART1->DR&0xffff;
        return (rec == data ? RET_OK : 0xffff);
    }

    return 0xffff;
}

static uint16_t UARTMON_InitPort_int()
{
    SetPinDir(P_Trst, 3);
    return RET_OK;
}

static uint16_t UARTMON_TargetReset_int()
{
    UARTMON_InitPort_int();

    SetPinDir(P_Trst, 1);
    RST_LO;
    sleep(50);
    RST_HI;
    sleep(50);

    return RET_OK;
}

// 0x00; Break

// 0x4a AA AA; Read Address
static uint16_t UARTMON_ReadMemory_int(uint32_t address, uint16_t *data)
{
    if (UARTMON_sendByte(0x4a)       == RET_OK &&
        UARTMON_sendByte(address>>8) == RET_OK &&
        UARTMON_sendByte(address)    == RET_OK  )
    {
        set_Timeout(500);
        while (!(USART_GetFlagStatus(USART1, USART_FLAG_RXNE)) && (get_Timeout() == RET_OK))  ;
        if (get_Timeout() == RET_OK)
        {

            *data = USART1->DR;
            return RET_OK;
        }
    }

    return 0xffff;
}


static uint16_t UARTMON_WriteMemory_int(uint32_t address, uint16_t data)
{
    if (UARTMON_sendByte(0x49)       == RET_OK &&
        UARTMON_sendByte(address>>8) == RET_OK &&
        UARTMON_sendByte(address)    == RET_OK &&
        UARTMON_sendByte(data)       == RET_OK  )
    {
        return RET_OK;
    }

    return 0xffff;
}


/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// External

void UARTMON_setup(const uint32_t TargetFreq)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING; // GPIO_Mode_IN_FLOATING; // GPIO_Mode_IPU
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_OD;  // Open drain since it's a one-wire implementation
    GPIO_Init(GPIOA, &GPIO_InitStructure);


    // External freq is divided by 4 to get the bus frequency, that is then divided by 256 to get the baudrate
    USART_InitStructure.USART_BaudRate = TargetFreq;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);

    // Enable clock output on MCO (GPIOA 8)
    // HSE is 8 (eight) MegaHertz
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    RCC_MCOConfig(RCC_MCO_HSE);
}

void UARTMON_TargetReset(const uint16_t *in, uint16_t *out)
{   out[0] =  UARTMON_TargetReset_int(); }
void UARTMON_InitPort(const uint16_t *in, uint16_t *out)
{   out[0] = UARTMON_InitPort_int();                      }


// [addr][addr],[len][len], [data]++
void UARTMON_WriteMemory(const uint16_t *in, uint16_t *out)
{
    uint32_t Address  = *(uint32_t *) &in[0];
    uint32_t Len      = *(uint32_t *) &in[2];
    uint16_t *dataptr =  (uint16_t *) &in[4];
    uint16_t retval   = RET_OK;

    // Hack in security
    if (Address == 0x10000)
    {
        while (Len && retval == RET_OK)
        {
            retval = UARTMON_sendByte(*dataptr++);
            Len--;
        }
    }
    else
    {
        while (Len && retval == RET_OK)
        {
            retval = UARTMON_WriteMemory_int(Address++,*dataptr++);
            Len--;
        }
    }

    out[0] = retval;
}

// In: [addr][addr],[len][len]
void UARTMON_ReadMemory(const uint16_t *in, uint16_t *out)
{
    uint16_t *dataptr =  (uint16_t *) &out[2];
    uint32_t Address  = *(uint32_t *) &in[0];
    uint32_t Len      = *(uint32_t *) &in[2];
    uint16_t retval   = RET_OK;
    uint32_t noRead   = 0;

    while (noRead < Len && retval == RET_OK)
    {
        retval = UARTMON_ReadMemory_int(Address++, &dataptr[noRead++]);
    }

    out[0] = retval;
    out[1] = 2 + (noRead>>1) + (noRead&1);
}





















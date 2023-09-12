#include "TAP/TAP_shared.h"

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_spi.h"

#include "hw_config.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"

volatile uint32_t usbrec = 0;
uint16_t receiveBuffer[ADAPTER_BUFzIN/2];
uint16_t sendBuffer[(ADAPTER_BUFzOUT/2)+2];

void EP1_IN_Callback() {}
void SOF_Callback() {}

uint16_t usb_sendData(const void *buffer)
{
    uint16_t *packet = (uint16_t *) buffer;
    uint32_t  length = packet[0] * 2;

    // There's a bug somewhere in there...
    uint8_t  PacketSize = (USB_DATA_SIZE == length) ? (USB_DATA_SIZE - 2) : USB_DATA_SIZE;

    while (length)
    {
        if (length <= PacketSize)
            PacketSize = length;

        length -= PacketSize;

        while (GetEPTxStatus(ENDP1) == EP_TX_VALID)   ;

        UserToPMABufferCopy((uint8_t *)packet, ENDP1_TXADDR, PacketSize);
        SetEPTxCount(ENDP1, PacketSize);
        SetEPTxValid(ENDP1);

        packet += PacketSize/2;
    }

    return RET_OK;
}

// TODO: Check timing and reset expectmorebytes / pointer if a certain amount of time has passed
void usb_receiveData()
{
    static uint8_t *bufferptr = (uint8_t *)&receiveBuffer[0];
    static uint32_t expectmorebytes = 0;
    static uint32_t completeLen = 0;

    if (usbrec > 0)
    {
        // printf("usb_receiveData(): Locked\n\r");
        // Adapter is busy.
        // Implement Mutex locking of USB output and answer host
        // With exception of "DO_ABANDON"; kill everything.
        // uint8_t tmp[USB_DATA_SIZE];
        // uint32_t rd = USB_SIL_Read(EP3_OUT, tmp);
    }
    else
    {
        uint32_t recd = USB_SIL_Read(EP3_OUT, bufferptr);

        // Start of packet frame
        if (expectmorebytes == 0)
        {
            completeLen = (*(uint16_t *) &bufferptr[0]) * 2;

            // Header is either malformed or we'll soon receive more data...
            if (completeLen > USB_DATA_SIZE)
                expectmorebytes = completeLen - USB_DATA_SIZE;
        }

        // In the middle of receiving a frame
        else
        {
            if (expectmorebytes >= recd)
                expectmorebytes -= recd;
            else
                expectmorebytes  = 0;
        }

        bufferptr += recd;

        if (expectmorebytes == 0)
        {
            bufferptr = (uint8_t *)&receiveBuffer[0];
            usbrec = completeLen;
        }

        SetEPRxValid(ENDP3);
    }
}

static void SPI_PreinitDMA()
{
    DMA_InitTypeDef DMA_InitStructure;

    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, DISABLE);

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    DMA_DeInit(DMA1_Channel4);
    DMA_DeInit(DMA1_Channel5);

    // Rx
    DMA_StructInit(&DMA_InitStructure);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPI2->DR;
    DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize         = 0;
    DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode               = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority           = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M                = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel4, &DMA_InitStructure);

    // TX
    DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Priority           = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralDST;
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);

    DMA1_Channel4->CCR &= ~DMA_CCR1_EN;
    DMA1_Channel5->CCR &= ~DMA_CCR1_EN;

    // printf("DMA1_Channel4->CCR: %08X\n\r", DMA1_Channel4->CCR);
    // printf("DMA1_Channel5->CCR: %08X\n\r", DMA1_Channel5->CCR);

    DMA1->ISR &= ~(DMA1_FLAG_TC4 | DMA1_FLAG_TC5);

    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, ENABLE);
}

#ifdef DEBUGPRINT
static const char *spiSpd[] = {
"SPI_BaudRatePrescaler_2",
"SPI_BaudRatePrescaler_4",
"SPI_BaudRatePrescaler_8",
"SPI_BaudRatePrescaler_16",
"SPI_BaudRatePrescaler_32",
"SPI_BaudRatePrescaler_64",
"SPI_BaudRatePrescaler_128",
"SPI_BaudRatePrescaler_256"
};
#endif


// SPI2 is apb1 (which is ran at 24 MHz (sysfreq / 2))
void InitSPI(const spi_cfg_t *cfg)
{
    SPI_I2S_DeInit(SPI2);
    SPI_InitTypeDef SPI_InitStructure;

    SPI_InitStructure.SPI_Direction     = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode          = SPI_Mode_Master;
    SPI_InitStructure.SPI_NSS           = SPI_NSS_Soft;
    SPI_InitStructure.SPI_CRCPolynomial = 0;

    SPI_InitStructure.SPI_FirstBit      = cfg->order    ? SPI_FirstBit_MSB : SPI_FirstBit_LSB;
    SPI_InitStructure.SPI_DataSize      = cfg->size     ? SPI_DataSize_16b : SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL          = cfg->polarity ? SPI_CPOL_High    : SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA          = cfg->phase    ? SPI_CPHA_2Edge   : SPI_CPHA_1Edge;

    if (cfg->frequency >= (24000000 / 2))
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    else if (cfg->frequency >= (24000000 / 4))
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    else if (cfg->frequency >= (24000000 / 8))
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
    else if (cfg->frequency >= (24000000 / 16))
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
    else if (cfg->frequency >= (24000000 / 32))
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
    else if (cfg->frequency >= (24000000 / 64))
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
    else if (cfg->frequency >= (24000000 / 128))
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
    else
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;

#ifdef DEBUGPRINT
    printf("Requested freq: %u.%u MHz\n\r", (u16)((u32)cfg->frequency/1000000),(u16)((u32)(cfg->frequency % 1000000)/100000));
    printf("Setting SPI to %s\n\r",  spiSpd[(SPI_InitStructure.SPI_BaudRatePrescaler>>3)&7]);
#endif

    SPI_Init(SPI2, &SPI_InitStructure);

    SPI_CalculateCRC(SPI2, DISABLE);
    SPI_Cmd(SPI2, ENABLE);

    SPI_PreinitDMA();
}

static void init_Timeout()
{
    NVIC_InitTypeDef NVIC_InitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_Cmd(TIM2,DISABLE);
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void init_debugUart()
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);
    USART_Cmd(USART2, ENABLE);
}

static void RCC_Configuration()
{
    RCC_APB2PeriphClockCmd(0x0101D, ENABLE); // GPIO A,B,C, SPI, AFIO for SPI

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB1PeriphClockCmd(0x24005, ENABLE); // Usart 2, spi, spi afio, more spi
}

static void InitSys()
{
    RCC_Configuration();
    init_debugUart(); // Debug uart
    init_Timeout();

    // Enable DWT timer
    if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk))
    {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
}

int main()
{
    InitSys();
    Set_System();
#ifdef BIGBOARD
    SetPinDir(2, 12, 1);
    USB_DIS_LO;
#endif
    Set_USBClock();
    USB_Interrupts_Config();
    USB_Init();

    TAP_InitPins();
	TAP_ResetState();

    uint8_t  *byteptr = (uint8_t  *) &receiveBuffer[0];

#ifdef DEBUGPRINT
    printf("adapter online\n\r");
#endif

	while(1)
	{
	    if (usbrec)
	    {
	        // We expect data in little-endian format.
	        // Host makes sure not to mix commands. First command in queue determines what rest is allowed
	        // Word[2] Contains command. Commands are split in categories of:
	        // 0x00xx: TAP
	        // 0x01xx: ???

	        switch (byteptr[5]) {
	            case 0x00:
	                TAP_Commands(receiveBuffer);
	                break;
	            default:
	                break;
	        }

	        usbrec = 0;
	    }
	}

	return 0;
}

void assert_failed(uint8_t* file, uint32_t line)
{   while (1) {} }

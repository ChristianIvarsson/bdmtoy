#include "common.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_tim.h"

static volatile uint32_t m_timeout;

#define DEBUGUART  USART2

void uart_putchar(char ch, FILE *f) {
	while(USART_GetFlagStatus(DEBUGUART, USART_FLAG_TXE) == RESET)  ;
	USART_SendData(DEBUGUART, ch);
}

char uart_getchar() {
	while(!USART_GetFlagStatus(DEBUGUART, USART_FLAG_RXNE))  ;
	return (DEBUGUART->DR)&0xFF;
}

int fputc(int ch, FILE *f) {
	if (ch == '\n') { fputc('\r', f); }
	while(USART_GetFlagStatus(DEBUGUART, USART_FLAG_TXE) == RESET)  ;
	USART_SendData(DEBUGUART, ch);
	return(ch);
}

void SetPinDir(const uint32_t port, const uint16_t pin, const uint8_t dir)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = 1 << pin;

    // Default to in, floating
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;

    if      (dir == 1) GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    else if (dir == 2) GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    else if (dir == 3) GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(((GPIO_TypeDef *) (GPIOA_BASE + (port*0x400))), &GPIO_InitStructure);
}

void sleep(const uint16_t ms)
{
    set_Timeout(ms);
    while (!m_timeout) ;
}

void set_Timeout(const uint16_t ms)
{
    TIM_Cmd(TIM2,DISABLE);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Period = ms-1;
    TIM_TimeBaseStructure.TIM_Prescaler = 48000;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

    m_timeout = 0;
    TIM_Cmd(TIM2,ENABLE);
}

uint32_t get_Timeout()
{
    return m_timeout;
}

/*
void disable_Timeout()
{
    TIM_Cmd(TIM2,DISABLE);
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
}*/

void TIM2_IRQHandler(void)
{
    TIM_Cmd(TIM2,DISABLE);
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    m_timeout = 1;
}

/*
void PrintIDdata(char *text, uint32_t ID) {
    printf(text);
    printf("%04X%04X\n\r", (uint16_t)( ID >> 16), (uint16_t)ID);
    printf("Rev:  %02X  ", (uint16_t)( ID >> 28)       );
    printf("DC :  %02X\n", (uint16_t)((ID >> 22)&0x03F));
    printf("PID: %03X  " , (uint16_t)((ID >> 12)&0x3FF));
    printf("MID: %03X\n\n",(uint16_t)((ID >>  1)&0x7FF));
}

// Byteswap
uint32_t SWAP(uint32_t in)
{   return ((in & 0xFF) << 24 | ((in >> 8) & 0xFF) << 16 | ((in >> 16) & 0xFF) << 8 | ((in >> 24) & 0xFF)); }

// Reverse bit-order
uint32_t REVERSE(uint32_t in) {
    uint32_t tmp = 0;
    uint8_t i;
    for (i = 0; i < 32; i++)
        tmp |= ((in >> i)&1) << (31 - i);
    return tmp;
}

void print16bit(uint16_t data) {
    printf("Data:");
    uint8_t i;
    for (i = 0; i < 16; i++) {
        if (!(i&0x7)) printf(" %u", (data >> (15 - i))&1);
        else          printf("%u",  (data >> (15 - i))&1);
    }
    printf("  (%04X)\n\r", data);
}

void printf32(char *text, uint32_t data) {
    printf(text);
    printf(" 0x%04X%04X\n\r", (uint16_t)(data>>16), (uint16_t)data);
}
*/

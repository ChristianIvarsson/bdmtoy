#ifndef __SUPFUNC_H__
#define __SUPFUNC_H__

void     uart_putchar(char c, FILE *stream);
char     uart_getchar(/*FILE *stream*/);
void     SetPinDir(const uint32_t port, const uint16_t pin, const uint8_t dir);

void     sleep      (const uint16_t ms);
void     set_Timeout(const uint16_t ms);
uint32_t get_Timeout();
// void     disable_Timeout();

/*
uint32_t SWAP(uint32_t in);
void     PrintIDdata(char *text, uint32_t ID);
void     printf32(char *text, uint32_t data);
*/

#endif

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Abstraction layer for CPU32
#ifndef __CPU32REQUESTS_H
#define __CPU32REQUESTS_H
#ifdef __cplusplus 
extern "C" {
#endif
#include "core_requests.h"

// 0 = PC
// 1 = PCC?
// 8 = Tempreg A
// 9 = Fault Address Reg
// A = Vector Base reg
// B = Status reg
// C = User Stack Pointer
// D = Supervisor Stack Pointer
// E = SFC
// F = DFC

#define CPU32_SREG_PC    00
#define CPU32_SREG_PCC   01
#define CPU32_SREG_TMPA  08
#define CPU32_SREG_FAR   09

#define CPU32_SREG_VBR   10
#define CPU32_SREG_STS   11
#define CPU32_SREG_USP   12
#define CPU32_SREG_SSP   13

#define CPU32_SREG_SFC   14
#define CPU32_SREG_DFC   15

void **CPU32_ReadSREG  (uint16_t Reg);
void **CPU32_ReadDREG  (uint16_t Reg);
void **CPU32_ReadAREG  (uint16_t Reg);

void **CPU32_WriteSREG (uint16_t Reg, uint32_t Data);
void **CPU32_WriteDREG (uint16_t Reg, uint32_t Data);
void **CPU32_WriteAREG (uint16_t Reg, uint32_t Data);


uint32_t CPU32_PrintRegSummary();

#ifdef __cplusplus 
}
#endif
#endif
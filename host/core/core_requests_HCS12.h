/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Abstraction layer for HCS12
#ifndef __HCS12REQUESTS_H
#define __HCS12REQUESTS_H
#ifdef __cplusplus 
extern "C" {
#endif
#include "core_requests.h"

#define HCS12_ReadNext()   TAP_ReadRegWord(0x62)
#define HCS12_ReadPC()     TAP_ReadRegWord(0x63)
#define HCS12_ReadD()      TAP_ReadRegWord(0x64)
#define HCS12_ReadX()      TAP_ReadRegWord(0x65)
#define HCS12_ReadY()      TAP_ReadRegWord(0x66)
#define HCS12_ReadSP()     TAP_ReadRegWord(0x67)

#define HCS12_WriteNEXT(a) TAP_WriteRegWord(0x42,(a))
#define HCS12_WritePC(a)   TAP_WriteRegWord(0x43,(a))
#define HCS12_WriteD(a)    TAP_WriteRegWord(0x44,(a))
#define HCS12_WriteX(a)    TAP_WriteRegWord(0x45,(a))
#define HCS12_WriteY(a)    TAP_WriteRegWord(0x46,(a))
#define HCS12_WriteSP(a)   TAP_WriteRegWord(0x47,(a))

void   **HCS12_ReadBDMAddress  (uint8_t Address);
void   **HCS12_WriteBDMAddress (uint8_t Address, uint8_t Data);

uint32_t HCS12_WaitBDM();
uint32_t HCS12_PrintRegSummary();
uint32_t HCS12_PrintStackContents(uint8_t noItems);

#ifdef __cplusplus 
}
#endif
#endif
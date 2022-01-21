/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Abstraction layer for JTAG
#ifndef __JTAGREQUESTS_H
#define __JTAGREQUESTS_H
#ifdef __cplusplus 
extern "C" {
#endif
#include "core_requests.h"

void *JTAG_ReadIREG     (uint16_t noBits);
void *JTAG_WriteIREG_w  (uint16_t noBits, uint16_t Data);

void *JTAG_ReadDREG     (uint16_t noBits);
void *JTAG_WriteDREG_w  (uint16_t noBits, uint16_t Data);


#define JTAG_ResetTAP() TAP_TargetReady()

// #define HCS12_WriteSP(a)   TAP_WriteRegWord(0x47,(a))



#ifdef __cplusplus 
}
#endif
#endif
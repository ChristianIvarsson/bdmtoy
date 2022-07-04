/// This file contains things that are of interest to more functions than their corresponding module
#ifndef __TAPSHARED_H__
#define __TAPSHARED_H__

#include "../common.h"
#include "BDM_HC12/BDMs_high.h"
#include "BDM_OLD/BDMo_high.h"
#include "BDM_NEW/BDMn_high.h"
#include "NEXUS/NEXUS_high.h"
#include "JTAG/JTAG_high.h"
#include "UART_MON/UARTMON_high.h"

// Misc shared stuff used by sub-modes
uint16_t *TAP_RequestData (const uint32_t Address, const uint32_t Len);
void      TAP_UpdateStatus(const uint16_t status , const uint16_t flag);
uint32_t  TAP_calcDelay   (const uint32_t inDelay, const float targetTicks, const float TargetFreq);
void      TAP_PreciseDelay(const uint32_t ticks);

void TAP_ResetState();
void TAP_InitPins();
void TAP_Commands(const void *bufin);

// Why..? Just why, Motorola?
#define BIT_REVERSE(Width,Bit) (1 << (((Width)-1)-(Bit)))

#endif

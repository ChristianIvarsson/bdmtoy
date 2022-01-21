/// This file contains things that are of interest to more functions than their corresponding module
#ifndef __TAPSHARED_H__
#define __TAPSHARED_H__

#include "../../common.h"
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

/*
void TAP_RestoreOwner();
enum TAP_GlobalBits
{
    TAP_ONLINE     = 0x0001, // Pointers has been installed, target has been reset and initialized to receive commands

    // Target information
    // TAP_BIGENDIAN  = 0x0020, // Target is big endian. Only affects a select few internal functions when the adapter has to send processor instructions on its own
    TAP_COMPRESSED = 0x0021, // Target is running in compact mode. Only affects a select few internal functions when the adapter has to send processor instructions on its own

    TAP_ABORT      = 0x8000, // Can only be set by the host computer. Abort whatever we're working on.
};
*/

struct {
    uint32_t DriveFreq; // ..

} TAP_Configs;

////////////////////////////////////////////////////////////////////////////////////
///////////////////////// Pay close attention! /////////////////////////////////////

// This is beyond dangerous. One missed pointer, one borked typedef, one missed parameter etc and the MCU _WILL_ crash!
// Think several times before poking around with these. This applies.. ESPECIALLY to me...
// Reason for doing it this way is that it's fairly simple to add support for more protocols without messing around with the existing functions.



/// Dynamic pointers to functions.
// "TAP_ResetState" takes length into account so there is no need to patch it in case new pointers are put in here.
struct {

    void *DYN_TargetInitPort_pntr;

    void *DYN_TargetReset_pntr;
    void *DYN_TargetReady_pntr;

    void *DYN_TargetStart_pntr;
    void *DYN_TargetStop_pntr;
    void *DYN_TargetStatus_pntr;

    void *DYN_WriteRegister_pntr;
    void *DYN_ReadRegister_pntr;

    void *DYN_WriteMemory_pntr;
    void *DYN_ReadMemory_pntr;

    void *DYN_FillMemory_pntr;
    void *DYN_DumpMemory_pntr;

    void *DYN_AssistFlash_pntr;

    void *DYN_ExecIns_pntr;
    void *DYN_RelTar_pntr;

} TAP_funcPntrs;

#endif

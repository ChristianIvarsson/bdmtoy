/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Abstraction layer for JTAG
#ifdef __cplusplus 
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "core.h"
#include "core_worker.h"
#include "core_requests.h"

void *JTAG_ReadIREG(uint16_t noBits)
{
    static uint16_t arr[TAP_ReadReg_sz];

    arr[0] = TAP_DO_READREGISTER;
    arr[1] = TAP_ReadReg_sz;
    arr[2] = JTAG_IREG;
    arr[3] = noBits;

    return &arr[0];
}

// Write up to 16 bits, 1 word, to ireg
void *JTAG_WriteIREG_w(uint16_t noBits, uint16_t Data)
{
    static uint16_t arr[TAP_WriteReg_sz];

    arr[0] = TAP_DO_WRITEREGISTER;
    arr[1] = TAP_WriteReg_sz;
    arr[2] = JTAG_IREG;
    arr[3] = noBits;
    arr[4] = Data;

    return &arr[0];
}

void *JTAG_ReadDREG(uint16_t noBits)
{
    static uint16_t arr[TAP_ReadReg_sz];

    arr[0] = TAP_DO_READREGISTER;
    arr[1] = TAP_ReadReg_sz;
    arr[2] = JTAG_DREG;
    arr[3] = noBits;

    return &arr[0];
}


// Write up to 16 bits, 1 word, to dreg
void *JTAG_WriteDREG_w(uint16_t noBits, uint16_t Data)
{
    static uint16_t arr[TAP_WriteReg_sz];

    arr[0] = TAP_DO_WRITEREGISTER;
    arr[1] = TAP_WriteReg_sz;
    arr[2] = JTAG_DREG;
    arr[3] = noBits;
    arr[4] = Data;

    return &arr[0];
}



#ifdef __cplusplus 
}
#endif

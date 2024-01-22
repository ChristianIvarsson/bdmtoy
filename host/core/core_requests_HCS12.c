/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Abstraction layer for HCS12
#ifdef __cplusplus 
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "core.h"
#include "core_worker.h"
#include "core_requests.h"
#include "core_requests_HCS12.h"

// Effing weird controller...
void **HCS12_ReadBDMAddress(uint8_t Address)
{
    return TAP_ReadByte(0x1FF00 + Address);
}

void **HCS12_WriteBDMAddress(uint8_t Address, uint8_t Data)
{
    return TAP_WriteByte(0x1FF00 + Address, Data);
}

// TODO: Add timeout
uint32_t HCS12_WaitBDM()
{
    uint32_t iterations = 0;
    uint16_t *ptr;

    do{ ptr = wrk_requestData( HCS12_ReadBDMAddress(1) );
        if (!ptr) {
            core_castText("Failed to read bdm status");
            return 0xFFFF;
        }
        iterations++;
    } while ( !(ptr[2]&0x40) );

    // core_castText("iterations: %u", iterations);
    return RET_OK;
}

uint32_t HCS12_PrintRegSummary()
{
    uint16_t rPC,rSP,rD,rX,rY;
    uint16_t *ptr;

    // PC, SP
    ptr = wrk_requestData( HCS12_ReadPC() );
    if (!ptr) return 0xFFFF;
    rPC = ptr[2];
    ptr = wrk_requestData( HCS12_ReadSP() );
    if (!ptr) return 0xFFFF;
    rSP = ptr[2];

    // D, X, Y
    ptr = wrk_requestData( HCS12_ReadD() );
    if (!ptr) return 0xFFFF;
    rD = ptr[2];
    ptr = wrk_requestData( HCS12_ReadX() );
    if (!ptr) return 0xFFFF;
    rX = ptr[2];
    ptr = wrk_requestData( HCS12_ReadY() );
    if (!ptr) return 0xFFFF;
    rY = ptr[2];

    core_castText("D : 0x%04X  X : 0x%04X  Y : 0x%04X",rD,rX,rY);
    core_castText("PC: 0x%04X  SP: 0x%04X", rPC, rSP);

    return RET_OK;
}

uint32_t HCS12_PrintStackContents(uint8_t noItems)
{
    uint16_t *ptr;
    uint16_t Address;

    ptr = wrk_requestData( HCS12_ReadSP() );
    if (!ptr) return 0xFFFF;

    Address = ptr[2];
    Address += (noItems-1)*2;

    while (noItems > 1)
    {
        ptr = wrk_requestData( TAP_ReadDword(Address) );
        if (!ptr) return 0xFFFF;

        core_castText("(%04X) %3d: %04X", Address, (noItems-1)*2, ptr[2]);
        Address -= 2;
        noItems--;
    }
    ptr = wrk_requestData( TAP_ReadDword(Address) );
    if (!ptr) return 0xFFFF;
    core_castText("(%04X)  SP: %04X", Address, ptr[2]);
    return 0;
}

#ifdef __cplusplus 
}
#endif

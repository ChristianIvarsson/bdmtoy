#ifndef __REQUESTS_CPU32_H__
#define __REQUESTS_CPU32_H__

#include "requests.h"

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

class requests_cpu32
    : public requests
{
public:
    uint16_t *readSystemRegister(uint16_t Reg) {
        return readRegister(0x2580 + (Reg & 0xF), sizeDword);
    }
    uint16_t *writeSystemRegister(uint16_t Reg, uint32_t Data) {
        return writeRegister(0x2480 + (Reg & 0xF), Data, sizeDword);
    }

    uint16_t *readDataRegister(uint16_t Reg) {
        return readRegister(0x2180 + (Reg & 7), sizeDword);
    }
    uint16_t *writeDataRegister(uint16_t Reg, uint32_t Data) {
        return writeRegister(0x2080 + (Reg & 7), Data, sizeDword);
    }

    uint16_t *readAddressRegister(uint16_t Reg) {
        return readRegister(0x2188 + (Reg & 7), sizeDword);
    }
    uint16_t *writeAddressRegister(uint16_t Reg, uint32_t Data) {
        return writeRegister(0x2088 + (Reg & 7), Data, sizeDword);
    }
};

#endif

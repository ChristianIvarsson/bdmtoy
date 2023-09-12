/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Abstraction layer for CPU32
#ifdef __cplusplus 
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "core.h"
#include "core_worker.h"
#include "core_requests.h"

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Register reads

void **CPU32_ReadSREG(uint16_t Reg)
{
	return TAP_ReadRegDword(0x2580 + (Reg&0xF));
}

void **CPU32_ReadDREG(uint16_t Reg)
{
    return TAP_ReadRegDword(0x2180 + (Reg&7));
}

void **CPU32_ReadAREG(uint16_t Reg)
{
    return TAP_ReadRegDword(0x2188 + (Reg&7));
}

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Register writes

void **CPU32_WriteSREG(uint16_t Reg, uint32_t Data)
{
	return TAP_WriteRegDword(0x2480 + (Reg&0xF), Data);
}

void **CPU32_WriteDREG(uint16_t Reg, uint32_t Data)
{
    return TAP_WriteRegDword(0x2080 + (Reg&7), Data);
}

void **CPU32_WriteAREG(uint16_t Reg, uint32_t Data)
{
    return TAP_WriteRegDword(0x2088 + (Reg&7), Data);
}

uint32_t CPU32_PrintRegSummary()
{
	uint32_t i;
	uint16_t *ptr;

	uint32_t regs[16] = { 0 };

	for (i = 0; i < 8; i++)
	{
		ptr = wrk_requestData(CPU32_ReadDREG(i));
		if (ptr)
			regs[i] = *(uint32_t*)&ptr[2];
	}

	for (i = 0; i < 8; i++)
	{
		ptr = wrk_requestData(CPU32_ReadAREG(i));
		if (ptr)
			regs[8 + i] = *(uint32_t*)&ptr[2];
	}

	core_castText("D:  %08x %08x %08x %08x %08x %08x %08x %08x", regs[ 0], regs[ 1], regs[ 2], regs[ 3], regs[ 4], regs[ 5], regs[ 6], regs[ 7]);
	core_castText("A:  %08x %08x %08x %08x %08x %08x %08x %08x", regs[ 8], regs[ 9], regs[10], regs[11], regs[12], regs[13], regs[14], regs[15]);

	ptr = wrk_requestData(CPU32_ReadSREG(0));
	if (ptr)
	{
		core_castText("PC: %08x", *(uint32_t*)&ptr[2]);
	}

	return 0;
}

#ifdef __cplusplus 
}
#endif

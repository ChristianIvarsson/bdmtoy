#ifndef __TARGET_H
#define __TARGET_H
#ifdef __cplusplus 
extern "C" {
#endif

#include "../core.h"
#include "../core_worker.h"
#include "../core_requests.h"

// Generic functions
uint32_t flashGeneric();
uint32_t dumpGenericLE        (uint32_t Start, uint32_t Length); // Dump in little endian format / byte transfers
uint32_t dumpGenericBE2       (uint32_t Start, uint32_t Length); // Dump in big endian format (Words)
uint32_t dumpGenericBE4       (uint32_t Start, uint32_t Length); // Dump in big endian format (Dwords)


// Oldschool BDM, CPU32 / 68k
uint32_t initTrionic5();
uint32_t initTrionic7();
uint32_t initT8main();
uint32_t initT8mcp();

uint32_t flashTrionic         (uint32_t Start, uint32_t Length, void *buffer);
uint32_t flashMCP             (uint32_t Start, uint32_t Length, void *buffer);
uint32_t dumpMCP              (uint32_t Start, uint32_t Length);


// Newschool BDM, PowerPC
uint32_t initEDC16C39();

uint32_t flashC39             (uint32_t Start, uint32_t Length, void *buffer);

uint32_t writeeepC39          (uint32_t Start, uint32_t Length, void *buffer);
uint32_t readeepC39           (uint32_t Start, uint32_t Length);


// sBDM, HCS12 etc
uint32_t initSID95_MY0405();
uint32_t initSIU95();

uint32_t flashSID95           (uint32_t Start, uint32_t Length, void *buffer);
uint32_t dumpSID95            (uint32_t Start, uint32_t Length);

uint32_t writeeepSID95        (uint32_t Start, uint32_t Length, void *buffer);
uint32_t readeepSID95         (uint32_t Start, uint32_t Length);


// In progress / toys
uint32_t initBMWcluster();
uint32_t dumpBMWcluster       (uint32_t Start, uint32_t Length);
uint32_t runSRAM_BMWcluster   (uint32_t Address, uint32_t Length, void *buffer);

uint32_t initMPC5566();
uint32_t MPC5566_play();
uint32_t dumpSRAM_mpc5566 (uint32_t Start, uint32_t Length);
uint32_t nexusDump        (uint32_t Start, uint32_t Length);



// Misc bits and bobs..
uint32_t secureEraseHCS12(uint32_t clockFreq);
uint32_t initBasic_HCS12();









uint32_t initUARTMON();
uint32_t dumpUARTMON       (uint32_t Start, uint32_t Length);







#ifdef __cplusplus 
}
#endif
#endif

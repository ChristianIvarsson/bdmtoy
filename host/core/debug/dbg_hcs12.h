#ifndef DBG_HCS12_H
#define DBG_HCS12_H
#ifdef __cplusplus 
extern "C" {
#endif

#include <stdint.h>
#include "../core.h"

// Test-registers for now.

static char dR0[]  = "R0 ";
static char dR1[]  = "R1 ";
static char dR2[]  = "R2 ";
static char dR3[]  = "R3 ";
static char dR4[]  = "R4 ";
static char dR5[]  = "R5 ";
static char dR6[]  = "R6 ";
static char dR7[]  = "R7 ";
static char dR8[]  = "R8 ";
static char dR9[]  = "R9 ";
static char dR10[] = "R10";
static char dR11[] = "R11";
static char dR12[] = "R12";
static char dR13[] = "R13";
static char dR14[] = "R14";
static char dR15[] = "R15";

static char dR16[] = "R16";
static char dR17[] = "R17";
static char dR18[] = "R18";
static char dR19[] = "R19";
static char dR20[] = "R20";
static char dR21[] = "R21";
static char dR22[] = "R22";
static char dR23[] = "R23";
static char dR24[] = "R24";
static char dR25[] = "R25";
static char dR26[] = "R26";
static char dR27[] = "R28";
static char dR28[] = "R28";
static char dR29[] = "R29";
static char dR30[] = "R30";
static char dR31[] = "R31";

#define debug_R32  \
    &dR0 , &dR1 , &dR2 , &dR3 , \
    &dR4 , &dR5 , &dR6 , &dR7 , \
    &dR8 , &dR9 , &dR10, &dR11, \
    &dR12, &dR13, &dR14, &dR15, \
    &dR16, &dR17, &dR18, &dR19, \
    &dR20, &dR21, &dR22, &dR23, \
    &dR24, &dR25, &dR26, &dR27, \
    &dR28, &dR29, &dR30, &dR31,

static const void *hcs12row[] =
{
    debug_R32
};

debug_td debghcs12 = 
{
    0,
    sizeof(hcs12row) / sizeof(void*),
    &hcs12row,
};

#ifdef __cplusplus 
}
#endif
#endif

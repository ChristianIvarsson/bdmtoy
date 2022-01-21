#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>

#include "core.h"

const void *debug_RegisterList(const uint32_t index)
{
    trgTemplate *targetstruc = (trgTemplate *) supportedTargets[index];
    debug_td    *trgDbg      = (debug_td    *) targetstruc->debug;

    if (index > numberoftargets_ || !index)
        return 0;

    if (!targetstruc->debug)
        return 0;

    return trgDbg->rowItems ? trgDbg->regRow : 0;
}

uint32_t debug_noRegisters(const uint32_t index)
{
    trgTemplate *targetstruc = (trgTemplate *) supportedTargets[index];
    debug_td    *trgDbg      = (debug_td    *) targetstruc->debug;

    if (index > numberoftargets_ || !index)
        return 0;

    if (!targetstruc->debug)
        return 0;

    return trgDbg->regRow ? trgDbg->rowItems : 0;
}

#ifdef __cplusplus 
}
#endif

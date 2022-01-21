
#include "csum_private.h"

uint32_t csum_Check(void *ptr, csTarg_t target, csRegion_t region)
{
    if (!region) {
        core_castText("csum_Check(): checksum.h specifically mentions NOT to pass checksumOK!");
        return RET_OK;
    }

    switch (target & (~255))
    {
        case fam16C39:
            return c39_Checksum(ptr, target, region);
        default:
            break;
    }

    core_castText("csum_Check(): I don't know this target..");
    return RET_OK;
}
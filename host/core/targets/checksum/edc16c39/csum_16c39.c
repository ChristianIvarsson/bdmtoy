
#include "csum_16c39_private.h"

uint32_t c39_Checksum(void *ptr, csTarg_t target, csRegion_t region)
{
    uint32_t *data = ptr;

    // Check if this thing even has data in the boot partition
    if (region & bootPartition) {
        if (!data[100/4] || data[100/4] == ~0) {
            core_castText("c39_Checksum(): Boot partition missing!");
            return bootPartition;
        }
    }

    switch (target)
    {
        case EDC16C39_95:
            return c39_csum_SAAB95(ptr, region);
        default:
            break;
    }

    core_castText("c39_Checksum(): I don't know this target");
    return RET_OK;
}
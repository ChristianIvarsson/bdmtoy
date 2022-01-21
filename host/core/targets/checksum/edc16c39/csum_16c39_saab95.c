
#include "csum_16c39_private.h"

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// Boot

// Figure out where the "patchsum" is located
static int32_t c39s95_retPatchaddress(uint32_t start, uint32_t end)
{
	uint32_t m_end = (end + 3) & 0xFFFFFFE0;

    if (start >= end) return -1;

    for ( ; start < m_end; start += 32)  {} // Large chunks
    for ( ; start < end  ; start +=  4)  {} // Remainder

	return (int32_t)(start - 4);
}

static uint32_t c39s95_csumBoot(void *ptr, uint32_t start, uint32_t end)
{
    uint32_t *ptr_  = ptr;

    // utl_checksum32block
}


// Figure out version, if we know how to checksum this, etc, etc..
static uint32_t c39s95_csumBootEntry(void *ptr)
{
    uint32_t *ptr_  = ptr;
    uint32_t  start = 0;

    // utl_checksum32block
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// Entry, public

uint32_t c39_csum_SAAB95(void *ptr, csRegion_t region)
{
    uint32_t retval = region;
    return c39s95_csumBootEntry(ptr);
}

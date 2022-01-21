#include <stdint.h>
#include <stdio.h>
#include "../core.h"

uint32_t utl_checksum(const void *data, uint32_t noBytes)
{
    const uint8_t *ptr = data;
    uint32_t checksum = 0;
    
    while (noBytes--)
        checksum += *ptr++;

    return checksum;
}

uint32_t utl_checksum32block(const void *data, uint32_t noBytes)
{
    const uint32_t *ptr = data;
    uint32_t checksum = 0;

    noBytes /= 4;
    
    while (noBytes--)
        checksum += *ptr++;

    return checksum;
}

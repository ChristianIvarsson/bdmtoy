#ifndef CSUM_16C39_PRIVATE_H
#define CSUM_16C39_PRIVATE_H
#ifdef __cplusplus 
extern "C" {
#endif

#include "../../../core.h"

typedef struct {
    uint32_t StartAddress;     // Where does this region start?
    uint32_t EndAddress;       // Where should it end?
    uint32_t DesiredStartsum;  // Initial checksum should be this before summing
    uint32_t DesiredEndsum;    // ..And this is what it wants the resulting sum to be after the whole region has been summed
} c39_csumcontainer_type1_t;


// SAAB 9-5
uint32_t c39_csum_SAAB95       (void *ptr, csRegion_t region);













#ifdef __cplusplus 
}
#endif
#endif

#ifndef CHECKSUM_H
#define CHECKSUM_H
#ifdef __cplusplus 
extern "C" {
#endif

// -> What should be checked / repaired?
// <- What failed?
typedef enum
{
    checksumOK     = 0x0000, // Don't pass this one, it should only be returned
    fullBinary     = 0xFFFF,

    bootPartition  = 1<<0x0,
    osPartition    = 1<<0x1,
} csRegion_t;

// ECU family
typedef enum
{
    famTx          = 0x000 << 8, // SAAB     Trionic
    fam16C39       = 0x001 << 8, // BOSCH    EDC16C39
} csFam_t;

// xxx yy
// xxx   : Family
//     yy: Specific model
typedef enum
{
    // Trionic family
    Trionic52            = famTx      | 0x00,
    Trionic55            = famTx      | 0x01,
    Trionic7             = famTx      | 0x02,
    Trionic8_Main        = famTx      | 0x03,
    Trionic8_MCP         = famTx      | 0x04,

    // EDC16C39 family
    EDC16C39_95          = fam16C39   | 0x00,
    EDC16C39_93          = fam16C39   | 0x01,
} csTarg_t;

// Return:
// 0 if it passed, otherwise a bitmask of what failed
uint32_t csum_Check    (void      *ptr,     // Data pointer 
                        csTarg_t   target,  // Target type, doh
                        csRegion_t region); // Which region?

uint32_t csum_Repair   (void      *ptr,
                        csTarg_t   target,
                        csRegion_t region);

#ifdef __cplusplus 
}
#endif
#endif

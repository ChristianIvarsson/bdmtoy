#ifndef __FLASH_PARTITIONS_H__
#define __FLASH_PARTITIONS_H__

typedef struct {
    const uint32_t        did;
    const uint32_t        count;
    const uint32_t *const partitions;
} flashpart_t;

typedef struct {
    const flashpart_t *const parts;
    const uint32_t           count;
} dids_t;

typedef struct {
    const dids_t x8parts;
    const dids_t x16parts;
    const dids_t x32parts;
} didcollection_t;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AMD

// 512k / 16
static const uint32_t am29f400bt[] = {
    0x10000, // End of Sector  0 + 1
    0x20000, // End of Sector  1 + 1
    0x30000, // End of Sector  2 + 1
    0x40000, // End of Sector  3 + 1
    0x50000, // End of Sector  4 + 1
    0x60000, // End of Sector  5 + 1
    0x70000, // End of Sector  6 + 1
    0x78000, // End of Sector  7 + 1
    0x7A000, // End of Sector  8 + 1
    0x7C000, // End of Sector  9 + 1
    0x80000, // End of Sector 10 + 1
};

static const flashpart_t amd_x16[] = {
    { 0x00002223, sizeof(am29f400bt) / sizeof(am29f400bt[0])                   , am29f400bt           },
};

static const didcollection_t amd_dids = {
    { nullptr },
    { amd_x16        , sizeof(amd_x16) / sizeof(amd_x16[0]) },
    { nullptr }
};

#endif

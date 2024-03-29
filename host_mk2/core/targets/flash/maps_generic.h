#ifndef __MAPS_GENERIC_H__
#define __MAPS_GENERIC_H__

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Generic maps

// One 64k partition
static const uint32_t one64k[] = {
    0x10000
};

// One 128k partition
static const uint32_t one128k[] = {
    0x20000
};

// One 256k partition
static const uint32_t one256k[] = {
    0x40000
};

// One 512k partition
static const uint32_t one512k[] = {
    0x80000
};

/////////////////////////////////
// 4k partitions

// 64k
static const uint32_t sixteen4k[] = {
    0x01000, 0x02000, 0x03000, 0x04000,
    0x05000, 0x06000, 0x07000, 0x08000,
    0x09000, 0x0A000, 0x0B000, 0x0C000,
    0x0D000, 0x0E000, 0x0F000, 0x10000
};

// 128k
static const uint32_t thirtytwo4k[] = {
    0x01000, 0x02000, 0x03000, 0x04000,
    0x05000, 0x06000, 0x07000, 0x08000,
    0x09000, 0x0A000, 0x0B000, 0x0C000,
    0x0D000, 0x0E000, 0x0F000, 0x10000,
    0x11000, 0x12000, 0x13000, 0x14000,
    0x15000, 0x16000, 0x17000, 0x18000,
    0x19000, 0x1A000, 0x1B000, 0x1C000,
    0x1D000, 0x1E000, 0x1F000, 0x20000
};

//256k
static const uint32_t sixtyfour4k[] = {
    0x01000, 0x02000, 0x03000, 0x04000,
    0x05000, 0x06000, 0x07000, 0x08000,
    0x09000, 0x0A000, 0x0B000, 0x0C000,
    0x0D000, 0x0E000, 0x0F000, 0x10000,
    0x11000, 0x12000, 0x13000, 0x14000,
    0x15000, 0x16000, 0x17000, 0x18000,
    0x19000, 0x1A000, 0x1B000, 0x1C000,
    0x1D000, 0x1E000, 0x1F000, 0x20000,
    0x21000, 0x22000, 0x23000, 0x24000,
    0x25000, 0x26000, 0x27000, 0x28000,
    0x29000, 0x2A000, 0x2B000, 0x2C000,
    0x2D000, 0x2E000, 0x2F000, 0x30000,
    0x31000, 0x32000, 0x33000, 0x34000,
    0x35000, 0x36000, 0x37000, 0x38000,
    0x39000, 0x3A000, 0x3B000, 0x3C000,
    0x3D000, 0x3E000, 0x3F000, 0x40000
};

//512k
static const uint32_t onetwentyeight4k[] = {
    0x01000, 0x02000, 0x03000, 0x04000,
    0x05000, 0x06000, 0x07000, 0x08000,
    0x09000, 0x0A000, 0x0B000, 0x0C000,
    0x0D000, 0x0E000, 0x0F000, 0x10000,
    0x11000, 0x12000, 0x13000, 0x14000,
    0x15000, 0x16000, 0x17000, 0x18000,
    0x19000, 0x1A000, 0x1B000, 0x1C000,
    0x1D000, 0x1E000, 0x1F000, 0x20000,
    0x21000, 0x22000, 0x23000, 0x24000,
    0x25000, 0x26000, 0x27000, 0x28000,
    0x29000, 0x2A000, 0x2B000, 0x2C000,
    0x2D000, 0x2E000, 0x2F000, 0x30000,
    0x31000, 0x32000, 0x33000, 0x34000,
    0x35000, 0x36000, 0x37000, 0x38000,
    0x39000, 0x3A000, 0x3B000, 0x3C000,
    0x3D000, 0x3E000, 0x3F000, 0x40000,

    0x41000, 0x42000, 0x43000, 0x44000,
    0x45000, 0x46000, 0x47000, 0x48000,
    0x49000, 0x4A000, 0x4B000, 0x4C000,
    0x4D000, 0x4E000, 0x4F000, 0x50000,
    0x51000, 0x52000, 0x53000, 0x54000,
    0x55000, 0x56000, 0x57000, 0x58000,
    0x59000, 0x5A000, 0x5B000, 0x5C000,
    0x5D000, 0x5E000, 0x5F000, 0x60000,
    0x61000, 0x62000, 0x63000, 0x64000,
    0x65000, 0x66000, 0x67000, 0x68000,
    0x69000, 0x6A000, 0x6B000, 0x6C000,
    0x6D000, 0x6E000, 0x6F000, 0x70000,
    0x71000, 0x72000, 0x73000, 0x74000,
    0x75000, 0x76000, 0x77000, 0x78000,
    0x79000, 0x7A000, 0x7B000, 0x7C000,
    0x7D000, 0x7E000, 0x7F000, 0x80000
};

/////////////////////////////////
// 16k partitions

static const uint32_t four16k[] = {
    0x04000, 0x08000, 0x0C000, 0x10000,
};

static const uint32_t eight16k[] = {
    0x04000, 0x08000, 0x0C000, 0x10000,
    0x14000, 0x18000, 0x1C000, 0x20000
};

static const uint32_t sixteen16k[] = {
    0x04000, 0x08000, 0x0C000, 0x10000,
    0x14000, 0x18000, 0x1C000, 0x20000,
    0x24000, 0x28000, 0x2C000, 0x30000,
    0x34000, 0x38000, 0x3C000, 0x40000
};

/////////////////////////////////
// 32k partitions

static const uint32_t two32k[] = {
    0x008000, 0x010000
};

static const uint32_t four32k[] = {
    0x008000, 0x010000, 0x018000, 0x020000
};

/* ( To silence unused warning )
static const uint32_t eight32k[] = {
    0x008000, 0x010000, 0x018000, 0x020000,
    0x028000, 0x030000, 0x038000, 0x040000
};  
static const uint32_t sixteen32k[] = {
    0x008000, 0x010000, 0x018000, 0x020000,
    0x028000, 0x030000, 0x038000, 0x040000,
    0x048000, 0x050000, 0x058000, 0x060000,
    0x068000, 0x070000, 0x078000, 0x080000
};*/

#endif

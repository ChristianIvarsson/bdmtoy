/* This code is cheating in so many ways!
 *
 * 1: It will ONLY work on little endian architectures.
 * 2: Data should preferably start from a 4-byte aligned address
 * 3: More crap I can't remember
 */
#include <stdint.h>
#include <stdio.h>
#include "../core.h"

#define  FG(B, C, D)  ( (D) ^ ( (B) & ( (C) ^ (D) ) ) )
#define   H(B, C, D)  ( ( (B) ^ (C) ) ^ (D) )
#define   I(B, C, D)    ( (C) ^ ( (B) |~(D) ) )

#define trnsf(f, a, b, c, d, n, dat, key, shft) \
    (a) += f( (b), (c), (d) ) + (dat) + (key);   \
    (a)  = ((a) << (shft) | ( (a) & 0xFFFFFFFF ) >> (32- (shft)) ) + (n);

typedef struct {
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
} md5k_t;

static void md5_InitKeys(md5k_t *m) {
    m->a = 0x67452301;
    m->b = 0xefcdab89;
    m->c = 0x98badcfe;
    m->d = 0x10325476;
}

static void md5_Hash(md5k_t *m, const void *data, uint_fast32_t Length)
{
    const uint32_t *pntr = (uint32_t *) data;
    uint32_t A, B, C, D;

    do {
        A = m->a, B = m->b;
        C = m->c, D = m->d;

        trnsf(FG, A, B, C, D, B, pntr[ 0], 0xd76aa478,  7)
        trnsf(FG, D, A, B, C, A, pntr[ 1], 0xe8c7b756, 12)
        trnsf(FG, C, D, A, B, D, pntr[ 2], 0x242070db, 17)
        trnsf(FG, B, C, D, A, C, pntr[ 3], 0xc1bdceee, 22)
        trnsf(FG, A, B, C, D, B, pntr[ 4], 0xf57c0faf,  7)
        trnsf(FG, D, A, B, C, A, pntr[ 5], 0x4787c62a, 12)
        trnsf(FG, C, D, A, B, D, pntr[ 6], 0xa8304613, 17)
        trnsf(FG, B, C, D, A, C, pntr[ 7], 0xfd469501, 22)
        trnsf(FG, A, B, C, D, B, pntr[ 8], 0x698098d8,  7)
        trnsf(FG, D, A, B, C, A, pntr[ 9], 0x8b44f7af, 12)
        trnsf(FG, C, D, A, B, D, pntr[10], 0xffff5bb1, 17)
        trnsf(FG, B, C, D, A, C, pntr[11], 0x895cd7be, 22)
        trnsf(FG, A, B, C, D, B, pntr[12], 0x6b901122,  7)
        trnsf(FG, D, A, B, C, A, pntr[13], 0xfd987193, 12)
        trnsf(FG, C, D, A, B, D, pntr[14], 0xa679438e, 17)
        trnsf(FG, B, C, D, A, C, pntr[15], 0x49b40821, 22)

        trnsf(FG, A, D, B, C, B, pntr[ 1], 0xf61e2562,  5)
        trnsf(FG, D, C, A, B, A, pntr[ 6], 0xc040b340,  9)
        trnsf(FG, C, B, D, A, D, pntr[11], 0x265e5a51, 14)
        trnsf(FG, B, A, C, D, C, pntr[ 0], 0xe9b6c7aa, 20)
        trnsf(FG, A, D, B, C, B, pntr[ 5], 0xd62f105d,  5)
        trnsf(FG, D, C, A, B, A, pntr[10], 0x02441453,  9)
        trnsf(FG, C, B, D, A, D, pntr[15], 0xd8a1e681, 14)
        trnsf(FG, B, A, C, D, C, pntr[ 4], 0xe7d3fbc8, 20)
        trnsf(FG, A, D, B, C, B, pntr[ 9], 0x21e1cde6,  5)
        trnsf(FG, D, C, A, B, A, pntr[14], 0xc33707d6,  9)
        trnsf(FG, C, B, D, A, D, pntr[ 3], 0xf4d50d87, 14)
        trnsf(FG, B, A, C, D, C, pntr[ 8], 0x455a14ed, 20)
        trnsf(FG, A, D, B, C, B, pntr[13], 0xa9e3e905,  5)
        trnsf(FG, D, C, A, B, A, pntr[ 2], 0xfcefa3f8,  9)
        trnsf(FG, C, B, D, A, D, pntr[ 7], 0x676f02d9, 14)
        trnsf(FG, B, A, C, D, C, pntr[12], 0x8d2a4c8a, 20)

        trnsf( H, A, B, C, D, B, pntr[ 5], 0xfffa3942,  4)
        trnsf( H, D, A, B, C, A, pntr[ 8], 0x8771f681, 11)
        trnsf( H, C, D, A, B, D, pntr[11], 0x6d9d6122, 16)
        trnsf( H, B, C, D, A, C, pntr[14], 0xfde5380c, 23)
        trnsf( H, A, B, C, D, B, pntr[ 1], 0xa4beea44,  4)
        trnsf( H, D, A, B, C, A, pntr[ 4], 0x4bdecfa9, 11)
        trnsf( H, C, D, A, B, D, pntr[ 7], 0xf6bb4b60, 16)
        trnsf( H, B, C, D, A, C, pntr[10], 0xbebfbc70, 23)
        trnsf( H, A, B, C, D, B, pntr[13], 0x289b7ec6,  4)
        trnsf( H, D, A, B, C, A, pntr[ 0], 0xeaa127fa, 11)
        trnsf( H, C, D, A, B, D, pntr[ 3], 0xd4ef3085, 16)
        trnsf( H, B, C, D, A, C, pntr[ 6], 0x04881d05, 23)
        trnsf( H, A, B, C, D, B, pntr[ 9], 0xd9d4d039,  4)
        trnsf( H, D, A, B, C, A, pntr[12], 0xe6db99e5, 11)
        trnsf( H, C, D, A, B, D, pntr[15], 0x1fa27cf8, 16)
        trnsf( H, B, C, D, A, C, pntr[ 2], 0xc4ac5665, 23)

        trnsf( I, A, B, C, D, B, pntr[ 0], 0xf4292244,  6)
        trnsf( I, D, A, B, C, A, pntr[ 7], 0x432aff97, 10)
        trnsf( I, C, D, A, B, D, pntr[14], 0xab9423a7, 15)
        trnsf( I, B, C, D, A, C, pntr[ 5], 0xfc93a039, 21)
        trnsf( I, A, B, C, D, B, pntr[12], 0x655b59c3,  6)
        trnsf( I, D, A, B, C, A, pntr[ 3], 0x8f0ccc92, 10)
        trnsf( I, C, D, A, B, D, pntr[10], 0xffeff47d, 15)
        trnsf( I, B, C, D, A, C, pntr[ 1], 0x85845dd1, 21)
        trnsf( I, A, B, C, D, B, pntr[ 8], 0x6fa87e4f,  6)
        trnsf( I, D, A, B, C, A, pntr[15], 0xfe2ce6e0, 10)
        trnsf( I, C, D, A, B, D, pntr[ 6], 0xa3014314, 15)
        trnsf( I, B, C, D, A, C, pntr[13], 0x4e0811a1, 21)
        trnsf( I, A, B, C, D, B, pntr[ 4], 0xf7537e82,  6)
        trnsf( I, D, A, B, C, A, pntr[11], 0xbd3af235, 10)
        trnsf( I, C, D, A, B, D, pntr[ 2], 0x2ad7d2bb, 15)
        trnsf( I, B, C, D, A, C, pntr[ 9], 0xeb86d391, 21)

        m->a += A, m->b += B;
        m->c += C, m->d += D;
        pntr += 16;

    } while (Length -= 64);
}

// This thing is currently kinda stupid and can ONLY hash in multiples of 64 bytes
uint32_t *utl_md5Hash(const void *data, const uint32_t noBytes)
{
    static uint32_t keys[4]; // Not the nicest solution but it must be preserved after the call
    uint8_t buf[64], i;
    md5k_t mkeys;

    if (noBytes&63) {
        core_castText("MD5 code is stupid and can't hash this length!");
        return 0;
    }

    md5_InitKeys(&mkeys);

    md5_Hash(&mkeys, data, noBytes);

    // Append 1 bit
    buf[0] = 0x80;

    for (i = 1; i < 64; i++)
        buf[i] = 0;

    *(uint32_t *) &buf[56] = noBytes << 3;
    md5_Hash(&mkeys, &buf, 64);

    keys[0] = ByteSwap(mkeys.a);
    keys[1] = ByteSwap(mkeys.b);
    keys[2] = ByteSwap(mkeys.c);
    keys[3] = ByteSwap(mkeys.d);

    return &keys[0];
}

// Helper for PowerPC targets (and more?) since current code won't finalize the hash
uint32_t *utl_md5FinalTarget(const uint32_t *keys, const uint32_t noBytes)
{
    static uint32_t m_keys[4]; // Not the nicest solution but it must be preserved after the call
    uint8_t buf[64], i;
    md5k_t mkeys;

    if (noBytes&63) {
        core_castText("MD5 code is stupid and can't hash this length!");
        return 0;
    }

    mkeys.a = keys[0];
    mkeys.b = keys[1];
    mkeys.c = keys[2];
    mkeys.d = keys[3];

    // Append 1 bit
    buf[0] = 0x80;

    for (i = 1; i < 64; i++)
        buf[i] = 0;

    *(uint32_t *) &buf[56] = noBytes << 3;
    md5_Hash(&mkeys, &buf, 64);

    m_keys[0] = ByteSwap(mkeys.a);
    m_keys[1] = ByteSwap(mkeys.b);
    m_keys[2] = ByteSwap(mkeys.c);
    m_keys[3] = ByteSwap(mkeys.d);

    return &m_keys[0];
}

// This is one of those "I'm lazy"-functions. Its only purpose is to be OR'ed with a format mask
uint32_t utl_compareMD5(const uint32_t *keysA, const uint32_t *keysB)
{
    if (keysA[0] != keysB[0]) return 1;
    if (keysA[1] != keysB[1]) return 1;
    if (keysA[2] != keysB[2]) return 1;
    if (keysA[3] != keysB[3]) return 1;
    return 0;
}

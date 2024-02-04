#ifndef __MD5_H__
#define __MD5_H__

// This needs a lot of work but it works well enough for what it's used for

typedef struct {
    uint32_t A;
    uint32_t B;
    uint32_t C;
    uint32_t D;
} md5k_t;

namespace crypto {

// Perform a full byteswap of a Dword
#define bs32(n) \
    ((n) << 24 | (( (n) >> 8) & 0xFF) << 16 | (((n) >> 16) & 0xFF) << 8 | (((n) >> 24) & 0xFF))

class md5 {

    #define  FG(B, C, D)  ( (D) ^ ( (B) & ( (C) ^ (D) ) ) )
    #define   H(B, C, D)  ( ( (B) ^ (C) ) ^ (D) )
    #define   I(B, C, D)    ( (C) ^ ( (B) |~(D) ) )

    #define trnsf(f, a, b, c, d, n, dat, key, shft) \
        (a) += f( (b), (c), (d) ) + (dat) + (key);   \
        (a)  = ((a) << (shft) | ( (a) & 0xFFFFFFFF ) >> (32- (shft)) ) + (n);

    static void initKeys( md5k_t *keys ) {
        keys->A = 0x67452301;
        keys->B = 0xefcdab89;
        keys->C = 0x98badcfe;
        keys->D = 0x10325476;
    }

    static void hashInt( md5k_t *keys, const void *source, size_t length ) {
        uint32_t dwBuf[ 64 / 4 ];
        uint8_t *buf = (uint8_t*)dwBuf;
        const uint8_t *src = (uint8_t*)source;

        do {
            uint32_t A = keys->A;
            uint32_t B = keys->B;
            uint32_t C = keys->C;
            uint32_t D = keys->D;

            // Implement big endian
            for ( uint32_t i = 0; i < 64; i++ )
                buf[ i ] = *src++;

            trnsf(FG, A, B, C, D, B, dwBuf[ 0], 0xd76aa478,  7)
            trnsf(FG, D, A, B, C, A, dwBuf[ 1], 0xe8c7b756, 12)
            trnsf(FG, C, D, A, B, D, dwBuf[ 2], 0x242070db, 17)
            trnsf(FG, B, C, D, A, C, dwBuf[ 3], 0xc1bdceee, 22)
            trnsf(FG, A, B, C, D, B, dwBuf[ 4], 0xf57c0faf,  7)
            trnsf(FG, D, A, B, C, A, dwBuf[ 5], 0x4787c62a, 12)
            trnsf(FG, C, D, A, B, D, dwBuf[ 6], 0xa8304613, 17)
            trnsf(FG, B, C, D, A, C, dwBuf[ 7], 0xfd469501, 22)
            trnsf(FG, A, B, C, D, B, dwBuf[ 8], 0x698098d8,  7)
            trnsf(FG, D, A, B, C, A, dwBuf[ 9], 0x8b44f7af, 12)
            trnsf(FG, C, D, A, B, D, dwBuf[10], 0xffff5bb1, 17)
            trnsf(FG, B, C, D, A, C, dwBuf[11], 0x895cd7be, 22)
            trnsf(FG, A, B, C, D, B, dwBuf[12], 0x6b901122,  7)
            trnsf(FG, D, A, B, C, A, dwBuf[13], 0xfd987193, 12)
            trnsf(FG, C, D, A, B, D, dwBuf[14], 0xa679438e, 17)
            trnsf(FG, B, C, D, A, C, dwBuf[15], 0x49b40821, 22)

            trnsf(FG, A, D, B, C, B, dwBuf[ 1], 0xf61e2562,  5)
            trnsf(FG, D, C, A, B, A, dwBuf[ 6], 0xc040b340,  9)
            trnsf(FG, C, B, D, A, D, dwBuf[11], 0x265e5a51, 14)
            trnsf(FG, B, A, C, D, C, dwBuf[ 0], 0xe9b6c7aa, 20)
            trnsf(FG, A, D, B, C, B, dwBuf[ 5], 0xd62f105d,  5)
            trnsf(FG, D, C, A, B, A, dwBuf[10], 0x02441453,  9)
            trnsf(FG, C, B, D, A, D, dwBuf[15], 0xd8a1e681, 14)
            trnsf(FG, B, A, C, D, C, dwBuf[ 4], 0xe7d3fbc8, 20)
            trnsf(FG, A, D, B, C, B, dwBuf[ 9], 0x21e1cde6,  5)
            trnsf(FG, D, C, A, B, A, dwBuf[14], 0xc33707d6,  9)
            trnsf(FG, C, B, D, A, D, dwBuf[ 3], 0xf4d50d87, 14)
            trnsf(FG, B, A, C, D, C, dwBuf[ 8], 0x455a14ed, 20)
            trnsf(FG, A, D, B, C, B, dwBuf[13], 0xa9e3e905,  5)
            trnsf(FG, D, C, A, B, A, dwBuf[ 2], 0xfcefa3f8,  9)
            trnsf(FG, C, B, D, A, D, dwBuf[ 7], 0x676f02d9, 14)
            trnsf(FG, B, A, C, D, C, dwBuf[12], 0x8d2a4c8a, 20)

            trnsf(H , A, B, C, D, B, dwBuf[ 5], 0xfffa3942,  4)
            trnsf(H , D, A, B, C, A, dwBuf[ 8], 0x8771f681, 11)
            trnsf(H , C, D, A, B, D, dwBuf[11], 0x6d9d6122, 16)
            trnsf(H , B, C, D, A, C, dwBuf[14], 0xfde5380c, 23)
            trnsf(H , A, B, C, D, B, dwBuf[ 1], 0xa4beea44,  4)
            trnsf(H , D, A, B, C, A, dwBuf[ 4], 0x4bdecfa9, 11)
            trnsf(H , C, D, A, B, D, dwBuf[ 7], 0xf6bb4b60, 16)
            trnsf(H , B, C, D, A, C, dwBuf[10], 0xbebfbc70, 23)
            trnsf(H , A, B, C, D, B, dwBuf[13], 0x289b7ec6,  4)
            trnsf(H , D, A, B, C, A, dwBuf[ 0], 0xeaa127fa, 11)
            trnsf(H , C, D, A, B, D, dwBuf[ 3], 0xd4ef3085, 16)
            trnsf(H , B, C, D, A, C, dwBuf[ 6], 0x04881d05, 23)
            trnsf(H , A, B, C, D, B, dwBuf[ 9], 0xd9d4d039,  4)
            trnsf(H , D, A, B, C, A, dwBuf[12], 0xe6db99e5, 11)
            trnsf(H , C, D, A, B, D, dwBuf[15], 0x1fa27cf8, 16)
            trnsf(H , B, C, D, A, C, dwBuf[ 2], 0xc4ac5665, 23)

            trnsf(I , A, B, C, D, B, dwBuf[ 0], 0xf4292244,  6)
            trnsf(I , D, A, B, C, A, dwBuf[ 7], 0x432aff97, 10)
            trnsf(I , C, D, A, B, D, dwBuf[14], 0xab9423a7, 15)
            trnsf(I , B, C, D, A, C, dwBuf[ 5], 0xfc93a039, 21)
            trnsf(I , A, B, C, D, B, dwBuf[12], 0x655b59c3,  6)
            trnsf(I , D, A, B, C, A, dwBuf[ 3], 0x8f0ccc92, 10)
            trnsf(I , C, D, A, B, D, dwBuf[10], 0xffeff47d, 15)
            trnsf(I , B, C, D, A, C, dwBuf[ 1], 0x85845dd1, 21)
            trnsf(I , A, B, C, D, B, dwBuf[ 8], 0x6fa87e4f,  6)
            trnsf(I , D, A, B, C, A, dwBuf[15], 0xfe2ce6e0, 10)
            trnsf(I , C, D, A, B, D, dwBuf[ 6], 0xa3014314, 15)
            trnsf(I , B, C, D, A, C, dwBuf[13], 0x4e0811a1, 21)
            trnsf(I , A, B, C, D, B, dwBuf[ 4], 0xf7537e82,  6)
            trnsf(I , D, A, B, C, A, dwBuf[11], 0xbd3af235, 10)
            trnsf(I , C, D, A, B, D, dwBuf[ 2], 0x2ad7d2bb, 15)
            trnsf(I , B, C, D, A, C, dwBuf[ 9], 0xeb86d391, 21)

            keys->A += A;
            keys->B += B;
            keys->C += C;
            keys->D += D;

        } while (length -= 64);
    }

public:
    md5() {}

    static void hash( md5k_t *keys, const void *buffer, uint32_t length ) {

        uint32_t dwBuf[64 / 4];
        uint8_t *buf  = (uint8_t*)dwBuf;
        uint32_t left = (length & 63);

        initKeys( keys );

        if ( length > 63 )
            hashInt( keys, buffer, length & ~63 );

        // Update base of buffer to new offset
        buffer = &((uint8_t*)buffer)[ length & ~63 ];

        for ( uint32_t i = 0; i < (length & 63); i++ )
            buf[ i ] = ((uint8_t*)buffer)[ i ];

        // Append 1 bit
        buf[ left++ ] = 0x80;

	    // We must have enough room for the len-bytes
	    if ( left > 56 ) {
	        for ( ; left < 64; left++ )
	            buf[ left ] = 0;
	        hashInt( keys, buf, 64 );
	        left = 0; 
	    }

	    for ( ; left < 64; left++ )
	        buf[ left ] = 0;

	    dwBuf[ 56 / 4 ] = ( length << 3 );
        hashInt( keys, buf, 64 );

        keys->A = bs32(keys->A);
        keys->B = bs32(keys->B);
        keys->C = bs32(keys->C);
        keys->D = bs32(keys->D);
    }
};
};
#endif

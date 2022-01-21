.global MD5Hash

# Temporary storage
tmpA: .long 0 /*  0 */
tmpB: .long 0 /*  4 */
tmpC: .long 0 /*  8 */
tmpD: .long 0 /*  C */

keyA: .long 0 /* 10 */
keyB: .long 0 /* 14 */
keyC: .long 0 /* 18 */
keyD: .long 0 /* 1C */

tmp1:  .long 0 /* 20 */
tmp2:  .long 0 /* 24 */

/* 0x28 */
Key:   .long 0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee
Key00: .long 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501 
Key01: .long 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be 
Key02: .long 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821

Key03: .long 0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa
Key04: .long 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8
Key05: .long 0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed
Key06: .long 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a

Key07: .long 0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c
Key08: .long 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70
Key09: .long 0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05
Key10: .long 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665

Key11: .long 0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039
Key12: .long 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1
Key13: .long 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1
Key14: .long 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391


# 07: Rotate
# 06: Data offs
# 04: key address
# 03: D
# 02: C
# 01: B
# 00: A
.macro _fullFa _b, _c, _d, _a, _dataOffs, _rotate

    ldab  #\_rotate
    stab     7,sp

    ldab  #\_dataOffs
    stab     6,sp

    ldab  #\_d*4
    stab     3,sp
    ldab  #\_c*4
    stab     2,sp
    ldab  #\_b*4
    stab     1,sp
    ldab  #\_a*4
    stab     0,sp

    # \_b*4, \_c*4, \_d*4
    jsr hashF
    jsr transform
.endm

.macro _fullFb _b, _c, _d, _a, _dataOffs, _rotate

    ldab  #\_rotate
    stab     7,sp

    ldab  #\_dataOffs
    stab     6,sp

    ldab  #\_d*4
    stab     3,sp
    ldab  #\_c*4
    stab     2,sp
    ldab  #\_b*4
    stab     1,sp
    ldab  #\_a*4
    stab     0,sp

    # \_b*4, \_c*4, \_d*4
    jsr hashF

    ldab  #\_c*4
    stab     1,sp
    jsr transform
.endm

.macro _fullH _b, _c, _d, _a, _dataOffs, _rotate

    ldab  #\_rotate
    stab     7,sp

    ldab  #\_dataOffs
    stab     6,sp

    ldab  #\_d*4
    stab     3,sp
    ldab  #\_c*4
    stab     2,sp
    ldab  #\_b*4
    stab     1,sp
    ldab  #\_a*4
    stab     0,sp

    # \_b*4, \_c*4, \_d*4
    jsr hashH
    jsr transform
.endm

.macro _fullI _b, _c, _d, _a, _dataOffs, _rotate

    ldab  #\_rotate
    stab     7,sp

    ldab  #\_dataOffs
    stab     6,sp

    ldab  #\_d*4
    stab     3,sp
    ldab  #\_c*4
    stab     2,sp
    ldab  #\_b*4
    stab     1,sp
    ldab  #\_a*4
    stab     0,sp

    # \_b*4, \_c*4, \_d*4
    jsr hashI
    jsr transform
.endm

# X: Base of keys etc
# Y: Data pointer

# 02: Number of iterations (16-bit word)
# sp:
.align 4
MD5Hash:

    # gcc-as has SERIOUS problems with figuring out branch/jump addresses so this is a hack..
    ldd #MD5Hash
    pshd

    # Make room for cache and parameters (And then some..)
    leas    -90,sp
    ldab    #0x28
    abx
    stx      4,sp
    leax -0x28,x

hashMore:

    # Copy data to local buffer
    ldab    #16
    pshx
    leax    10,sp

cpData:

    ldaa     3,y    
    staa     0,x
    ldaa     2,y
    staa     1,x
    ldaa     1,y
    staa     2,x
    ldaa     0,y
    staa     3,x

    leax     4,x
    leay     4,y
    decb
    bne cpData

    pulx

    jsr cpyKeys
# Step 0 - 15
    #        B C D A     data    rot
    _fullFa  1 2 3 0      0       7
    _fullFa  0 1 2 3      1      12
    _fullFa  3 0 1 2      2      17
    _fullFa  2 3 0 1      3      22

    _fullFa  1 2 3 0      4       7
    _fullFa  0 1 2 3      5      12
    _fullFa  3 0 1 2      6      17
    _fullFa  2 3 0 1      7      22

    _fullFa  1 2 3 0      8       7
    _fullFa  0 1 2 3      9      12
    _fullFa  3 0 1 2     10      17
    _fullFa  2 3 0 1     11      22

    _fullFa  1 2 3 0     12       7
    _fullFa  0 1 2 3     13      12
    _fullFa  3 0 1 2     14      17
    _fullFa  2 3 0 1     15      22

# Step 16 - 31
    _fullFb  3 1 2 0      1       5
    _fullFb  2 0 1 3      6       9
    _fullFb  1 3 0 2     11      14
    _fullFb  0 2 3 1      0      20

    _fullFb  3 1 2 0      5       5
    _fullFb  2 0 1 3     10       9
    _fullFb  1 3 0 2     15      14
    _fullFb  0 2 3 1      4      20

    _fullFb  3 1 2 0      9       5
    _fullFb  2 0 1 3     14       9
    _fullFb  1 3 0 2      3      14
    _fullFb  0 2 3 1      8      20

    _fullFb  3 1 2 0     13       5
    _fullFb  2 0 1 3      2       9
    _fullFb  1 3 0 2      7      14
    _fullFb  0 2 3 1     12      20

# Step 32 - 47
    _fullH   1 2 3 0      5       4
    _fullH   0 1 2 3      8      11
    _fullH   3 0 1 2     11      16
    _fullH   2 3 0 1     14      23

    _fullH   1 2 3 0      1       4
    _fullH   0 1 2 3      4      11
    _fullH   3 0 1 2      7      16
    _fullH   2 3 0 1     10      23

    _fullH   1 2 3 0     13       4
    _fullH   0 1 2 3      0      11
    _fullH   3 0 1 2      3      16
    _fullH   2 3 0 1      6      23

    _fullH   1 2 3 0      9       4
    _fullH   0 1 2 3     12      11
    _fullH   3 0 1 2     15      16
    _fullH   2 3 0 1      2      23

# Step 48 - 63
    _fullI   1 2 3 0      0       6
    _fullI   0 1 2 3      7      10
    _fullI   3 0 1 2     14      15
    _fullI   2 3 0 1      5      21

    _fullI   1 2 3 0     12       6
    _fullI   0 1 2 3      3      10
    _fullI   3 0 1 2     10      15
    _fullI   2 3 0 1      1      21

    _fullI   1 2 3 0      8       6
    _fullI   0 1 2 3     15      10
    _fullI   3 0 1 2      6      15
    _fullI   2 3 0 1     13      21

    _fullI   1 2 3 0      4       6
    _fullI   0 1 2 3     11      10
    _fullI   3 0 1 2      2      15
    _fullI   2 3 0 1      9      21

    pshx
    ldab  #4

nextKey:
    ldaa    3,x
    adda   19,x
    staa   19,x

    ldaa    2,x
    adca   18,x
    staa   18,x

    ldaa    1,x
    adca   17,x
    staa   17,x

    ldaa    0,x
    adca   16,x
    staa   16,x

    leax    4,x

    decb
    bne nextKey

    pulx

    pshy
    ldy     96,sp
    dey
    sty     96,sp
    beq hashDone
    puly
    
    # Return to our self instead of branching since, clearly, gcc-as is retarded...
    leas    90,sp
    rts

hashDone:
    leas    94,sp
rts






.align 4
cpyKeys:

    # A
    ldd    16,x
    std     0,x
    ldd    18,x
    std     2,x

    # B
    ldd    20,x
    std     4,x
    ldd    22,x
    std     6,x

    # C
    ldd    24,x
    std     8,x
    ldd    26,x
    std    10,x

    # D
    ldd    28,x
    std    12,x
    ldd    30,x
    std    14,x
rts













.align 4
_Leftrotate:

    lsl     3,x
    tpa
    tfr     a,b
    lsl     2,x
    tpa
    exg     a,b
    tap
    ldaa    2,x
    adca    #0
    staa    2,x
    lsl     1,x
    tpa
    exg     a,b
    tap
    ldaa    1,x
    adca    #0
    staa    1,x
    lsl     0,x
    tpa
    exg     a,b
    tap
    ldaa    0,x
    adca    #0
    staa    0,x
    exg     a,b
    tap
    ldaa    3,x
    adca    #0
    staa    3,x

    # Missing opcode..
    # dbne y, _Leftrotate
    dey
    bne _Leftrotate
rts
.align 4
#x: From
#y: to
_Addition:
    ldd  2,y
    addd 2,x
    std  2,y
    ldaa 1,y
    adca 1,x
    staa 1,y
    ldaa 0,y
    adca 0,x
    staa 0,y
rts
.align 4
# 10++: Data buffer
# 09: Rotate
# 08: Data offs
# 06: key address
# 05: D
# 04: C
# 03: B
# 02: A
# sp: 
transform:

    pshy /* 2 */
    pshx /* 0 */

    # Add data
    leay    32,x
    ldaa    12,sp
    ldab     #4
    mul

    # leax    32,sp
    leax    14,sp /* data ptr */
    abx
    jsr _Addition

    # Load destination, A, to destination register
    ldab     6,sp
    ldy      0,sp
    aby

    # Add key to A
    ldx     10,sp
    jsr _Addition
    leax     4,x
    stx     10,sp

    # Add temp value to A
    ldx      0,sp
    leax    32,x
    jsr _Addition

    # rotate
    ldx      0,sp /* Register table start     */
    ldab     6,sp /* Register table offset    */
    abx           /* Add offset               */
    ldd     #0
    ldab    13,sp /* /* No bits to rotate     */
    xgdy
    jsr _Leftrotate

    ldx      0,sp
    ldy      0,sp
    ldab     6,sp
    aby
    ldd #0
    ldab     7,sp
    abx
    jsr _Addition

    pulx
    puly
rts
.align 4
# X: Source
# Y: Destination to be ANDed with
_and:
    ldaa     3,x
    anda     3,y
    staa     3,y

    ldaa     2,x
    anda     2,y
    staa     2,y

    ldaa     1,x
    anda     1,y
    staa     1,y

    ldaa     0,x
    anda     0,y
    staa     0,y
rts





.align 4
#  y: Dest
# 03: Src b
# 02: Src a
# sp:
_xor:
    leax     3,x

    ldab     2,sp
    ldaa     b,x
    ldab     3,sp
    eora     b,x
    staa     3,y
    dex

    ldab     2,sp
    ldaa     b,x
    ldab     3,sp
    eora     b,x
    staa     2,y
    dex

    ldab     2,sp
    ldaa     b,x
    ldab     3,sp
    eora     b,x
    staa     1,y
    dex

    ldab     2,sp
    ldaa     b,x
    ldab     3,sp
    eora     b,x
    staa     0,y
rts


.align 4
# 05: D
# 04: C
# 03: B
# 02: A (Not used)
# sp:
hashF:
    
    pshy
    pshx

    # Temp register is at offset 32
    leay    32,x

    # We intend to store some temporary parameters

    # Add 4
    ldab     9,sp
    pshb

    # Add 5
    ldab     9,sp
    pshb

    #  _xor \_c, \_d, 32
    jsr _xor

    # Add 6
    ldab     9,sp
    abx

    # _and \_b,  32, 32
    jsr _and

    ldx      2,sp
    ldab    #32
    stab     0,sp

    # _xor \_d,  32, 32
    jsr _xor

    # Restore stack
    leas     2,sp
    pulx
    puly
rts
.align 4
# 05: D
# 04: C
# 03: B
# 02: A (Not used)
# sp:
hashH:
    
    pshy
    pshx

    # Temp register is at offset 32
    leay    32,x

    # We intend to store some temporary parameters

    # Add 4
    ldab     7,sp
    pshb

    # Add 5
    ldab     9,sp
    pshb

    #  _xor \_b, \_c, 32
    jsr _xor

    ldx      2,sp
    ldab    #32
    stab     0,sp

    ldab    11,sp
    stab     1,sp

    # _xor \_d,  32, 32
    jsr _xor

    # Restore stack
    leas     2,sp
    pulx
    puly
rts
.align 4

# 05: D
# 04: C
# 03: B
# 02: A (Not used)
# sp:
hashI:
    
    pshy
    pshx

    # Temp register is at offset 32
    leay    32,x

    ldab     9,sp
    abx

    #  _not \_d, 32
    jsr _not

    ldx      0,sp
    ldab     7,sp
    abx

    # _or  \_b, 32
    jsr _or

    ldx      0,sp
    ldab     8,sp
    pshb

    ldab    #32
    pshb

    # _xor \_c, 32, 32
    jsr _xor

    # Restore stack
    leas     2,sp
    pulx
    puly
rts

.align 4
# y: Dest
# X: Source
_not:
    ldaa     3,x
    coma
    staa     3,y
    ldaa     2,x
    coma
    staa     2,y
    ldaa     1,x
    coma
    staa     1,y
    ldaa     0,x
    coma
    staa     0,y
rts


_or:
    ldaa    3,x
    oraa    3,y
    staa    3,y

    ldaa    2,x
    oraa    2,y
    staa    2,y

    ldaa    1,x
    oraa    1,y
    staa    1,y

    ldaa    0,x
    oraa    0,y
    staa    0,y
rts

/*
.macro hashI _b, _c, _d
    _not \_d, 32
    _or  \_b, 32
    _xor \_c, 32, 32
.endm
*/
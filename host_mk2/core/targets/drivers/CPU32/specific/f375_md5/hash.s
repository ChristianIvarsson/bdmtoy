.global Hash

# This implementation does a few things back-to-front due to the target being big ending.
# In particular buffer index is completely reversed:

# Regular byteswap operation
#  0  1  2  3     4  5  6  7 .. 60 61 62 63   < read
#  3  2  1  0     7  6  5  4 .. 63 62 61 60   < in buffer
# idx 0          idx 1       .. idx 15        < Which index the source data is at

# Reversed byteswap operation
#  0  1  2  3     4  5  6  7 .. 60 61 62 63   < read
# 63 62 61 60    59 58 57 56 ..  3  2  1  0   < in buffer
# idx 15         idx 14      .. idx 0         < Which index the source data is at


# Various aliases
.set srcData, a0
.set tmpBuf , a1

.set dataLen, d0
.set tmpReg , d1

# Register aliases for key A - D

# Must be data registers and in order
.set  kA,  d2
.set  kB,  d3
.set  kC,  d4
.set  kD,  d5

# Must be address registers and in order
.set backA, a2
.set backB, a3
.set backC, a4
.set backD, a5

.macro _WRAP    _A, _B,   _TMP,    _DAT, _IDX,    _HASH,    _ROT
.if ( \_IDX != 0 )
    add.l    (\_IDX)*4(\_DAT), \_A
.else
    add.l    (\_DAT)     , \_A
.endif
    addi.l   #\_HASH     , \_A
    add.l    \_TMP       , \_A

# Perform left-rotate, but with a twist since it's REALLY slow. Sometimes it faster to swap and then roll
.if     ( \_ROT  <  8 )
    rol.l    #\_ROT      , \_A

.elseif ( \_ROT  < 16 )
    swap     \_A
    ror.l    #16 - \_ROT , \_A

.elseif ( \_ROT == 16 )
    swap     \_A

.elseif ( \_ROT  < 24 )
    swap     \_A
    rol.l    #\_ROT - 16 , \_A

.elseif ( \_ROT  < 32 )
    ror.l    #32 - \_ROT , \_A

.else
    # 32, do nothing
.endif
    add.l    \_B         , \_A
.endm

# FG with decrementing address index (less space)
.macro _FGmm  _A, _B, _C, _D,   _TMP,   _DAT, _IDX,    _HASH,    _ROT
    move.l   \_D         , \_TMP
    eor.l    \_C         , \_TMP
    and.l    \_B         , \_TMP
    eor.l    \_D         , \_TMP

    add.l    -(\_DAT)    , \_A
    addi.l   #\_HASH     , \_A
    add.l    \_TMP       , \_A

.if     ( \_ROT  <  8 )
    rol.l    #\_ROT      , \_A
.elseif ( \_ROT  < 16 )
    swap     \_A
    ror.l    #16 - \_ROT , \_A
.elseif ( \_ROT == 16 )
    swap     \_A
.elseif ( \_ROT  < 24 )
    swap     \_A
    rol.l    #\_ROT - 16 , \_A
.elseif ( \_ROT  < 32 )
    ror.l    #32 - \_ROT , \_A
.endif
    add.l    \_B         , \_A
.endm

# Regular FG
.macro _FG    _A, _B, _C, _D,   _TMP,   _DAT, _IDX,    _HASH,    _ROT
    move.l   \_D         , \_TMP
    eor.l    \_C         , \_TMP
    and.l    \_B         , \_TMP
    eor.l    \_D         , \_TMP
    _WRAP    \_A, \_C    , \_TMP,     \_DAT, \_IDX, \_HASH, \_ROT
.endm

.macro __H   _A, _B, _C, _D,   _TMP,   _DAT, _IDX,    _HASH,    _ROT
    move.l   \_B         , \_TMP
    eor.l    \_C         , \_TMP
    eor.l    \_D         , \_TMP
    _WRAP    \_A, \_B    , \_TMP,     \_DAT, \_IDX, \_HASH, \_ROT
.endm

.macro __I   _A, _B, _C, _D,   _TMP,   _DAT, _IDX,    _HASH,    _ROT
    move.l   \_D         , \_TMP
    not.l    \_TMP
    or.l     \_B         , \_TMP
    eor.l    \_C         , \_TMP
    _WRAP    \_A, \_B    , \_TMP,     \_DAT, \_IDX, \_HASH, \_ROT
.endm

# Stack frame
#
#  60: Buffer
#  44: Keys ( 4 registers, 16 bytes )
#  40: Length
#  36: Address
#  32: Return address
#   0: Register backup ( 8 registers, 32 bytes )

Hash:

    movem.l  kA-kD/backA-backD, -(sp)

    lea.l    44(sp)      , tmpBuf      /* Point at keys                               */
    movem.l  (tmpBuf)+   , backA-backD /* Load them and autoinc up to buffer location */
    move.l   40(sp)      , dataLen     /* Load length                                 */
    movea.l  36(sp)      , srcData     /* Load source address                         */

hashMore:
    moveq.l  #7          , tmpReg
    lea.l    64(tmpBuf)  , tmpBuf

# 8 bytes / loop seems to be the sweet spot
bufFill:
    move.b   (srcData)+  ,-(tmpBuf)
    move.b   (srcData)+  ,-(tmpBuf)
    move.b   (srcData)+  ,-(tmpBuf)
    move.b   (srcData)+  ,-(tmpBuf)

    move.b   (srcData)+  ,-(tmpBuf)
    move.b   (srcData)+  ,-(tmpBuf)
    move.b   (srcData)+  ,-(tmpBuf)
    move.b   (srcData)+  ,-(tmpBuf)

    dbra     tmpReg      , bufFill

# Service watchdog
    bsr.w    swsr

    # Restore Registers
    move.l   backA       , kA
    move.l   backB       , kB
    move.l   backC       , kC
    move.l   backD       , kD

    # VERY ugly trick since we know the first 16 hashes happens in descending order
    lea.l    64(tmpBuf)  , tmpBuf

    # # # # # STEP  0 # # # # #
    _FGmm kA, kB, kC, kD,   tmpReg,     tmpBuf, 15,    0xD76AA478,     7
    _FGmm kD, kA, kB, kC,   tmpReg,     tmpBuf, 14,    0xE8C7B756,    12
    _FGmm kC, kD, kA, kB,   tmpReg,     tmpBuf, 13,    0x242070DB,    17
    _FGmm kB, kC, kD, kA,   tmpReg,     tmpBuf, 12,    0xC1BDCEEE,    22
    _FGmm kA, kB, kC, kD,   tmpReg,     tmpBuf, 11,    0xF57C0FAF,     7
    _FGmm kD, kA, kB, kC,   tmpReg,     tmpBuf, 10,    0x4787C62A,    12
    _FGmm kC, kD, kA, kB,   tmpReg,     tmpBuf,  9,    0xA8304613,    17
    _FGmm kB, kC, kD, kA,   tmpReg,     tmpBuf,  8,    0xFD469501,    22
    _FGmm kA, kB, kC, kD,   tmpReg,     tmpBuf,  7,    0x698098D8,     7
    _FGmm kD, kA, kB, kC,   tmpReg,     tmpBuf,  6,    0x8B44F7AF,    12
    _FGmm kC, kD, kA, kB,   tmpReg,     tmpBuf,  5,    0xFFFF5BB1,    17
    _FGmm kB, kC, kD, kA,   tmpReg,     tmpBuf,  4,    0x895CD7BE,    22
    _FGmm kA, kB, kC, kD,   tmpReg,     tmpBuf,  3,    0x6B901122,     7
    _FGmm kD, kA, kB, kC,   tmpReg,     tmpBuf,  2,    0xFD987193,    12
    _FGmm kC, kD, kA, kB,   tmpReg,     tmpBuf,  1,    0xA679438E,    17
    _FGmm kB, kC, kD, kA,   tmpReg,     tmpBuf,  0,    0x49B40821,    22

    # # # # # STEP 16 # # # # #
    _FG   kA, kD, kB, kC,   tmpReg,     tmpBuf, 14,    0xf61e2562,     5
    _FG   kD, kC, kA, kB,   tmpReg,     tmpBuf,  9,    0xc040b340,     9
    _FG   kC, kB, kD, kA,   tmpReg,     tmpBuf,  4,    0x265e5a51,    14
    _FG   kB, kA, kC, kD,   tmpReg,     tmpBuf, 15,    0xe9b6c7aa,    20
    _FG   kA, kD, kB, kC,   tmpReg,     tmpBuf, 10,    0xd62f105d,     5
    _FG   kD, kC, kA, kB,   tmpReg,     tmpBuf,  5,    0x02441453,     9
    _FG   kC, kB, kD, kA,   tmpReg,     tmpBuf,  0,    0xd8a1e681,    14
    _FG   kB, kA, kC, kD,   tmpReg,     tmpBuf, 11,    0xe7d3fbc8,    20
    _FG   kA, kD, kB, kC,   tmpReg,     tmpBuf,  6,    0x21e1cde6,     5
    _FG   kD, kC, kA, kB,   tmpReg,     tmpBuf,  1,    0xc33707d6,     9
    _FG   kC, kB, kD, kA,   tmpReg,     tmpBuf, 12,    0xf4d50d87,    14
    _FG   kB, kA, kC, kD,   tmpReg,     tmpBuf,  7,    0x455a14ed,    20
    _FG   kA, kD, kB, kC,   tmpReg,     tmpBuf,  2,    0xa9e3e905,     5
    _FG   kD, kC, kA, kB,   tmpReg,     tmpBuf, 13,    0xfcefa3f8,     9
    _FG   kC, kB, kD, kA,   tmpReg,     tmpBuf,  8,    0x676f02d9,    14
    _FG   kB, kA, kC, kD,   tmpReg,     tmpBuf,  3,    0x8d2a4c8a,    20

    # # # # # STEP 32 # # # # #
    __H   kA, kB, kC, kD,   tmpReg,     tmpBuf, 10,    0xfffa3942,     4
    __H   kD, kA, kB, kC,   tmpReg,     tmpBuf,  7,    0x8771f681,    11
    __H   kC, kD, kA, kB,   tmpReg,     tmpBuf,  4,    0x6d9d6122,    16
    __H   kB, kC, kD, kA,   tmpReg,     tmpBuf,  1,    0xfde5380c,    23
    __H   kA, kB, kC, kD,   tmpReg,     tmpBuf, 14,    0xa4beea44,     4
    __H   kD, kA, kB, kC,   tmpReg,     tmpBuf, 11,    0x4bdecfa9,    11
    __H   kC, kD, kA, kB,   tmpReg,     tmpBuf,  8,    0xf6bb4b60,    16
    __H   kB, kC, kD, kA,   tmpReg,     tmpBuf,  5,    0xbebfbc70,    23
    __H   kA, kB, kC, kD,   tmpReg,     tmpBuf,  2,    0x289b7ec6,     4
    __H   kD, kA, kB, kC,   tmpReg,     tmpBuf, 15,    0xeaa127fa,    11
    __H   kC, kD, kA, kB,   tmpReg,     tmpBuf, 12,    0xd4ef3085,    16
    __H   kB, kC, kD, kA,   tmpReg,     tmpBuf,  9,    0x04881d05,    23
    __H   kA, kB, kC, kD,   tmpReg,     tmpBuf,  6,    0xd9d4d039,     4
    __H   kD, kA, kB, kC,   tmpReg,     tmpBuf,  3,    0xe6db99e5,    11
    __H   kC, kD, kA, kB,   tmpReg,     tmpBuf,  0,    0x1fa27cf8,    16
    __H   kB, kC, kD, kA,   tmpReg,     tmpBuf, 13,    0xc4ac5665,    23

    # # # # # STEP 48 # # # # #
    __I   kA, kB, kC, kD,   tmpReg,     tmpBuf, 15,    0xf4292244,     6
    __I   kD, kA, kB, kC,   tmpReg,     tmpBuf,  8,    0x432aff97,    10
    __I   kC, kD, kA, kB,   tmpReg,     tmpBuf,  1,    0xab9423a7,    15
    __I   kB, kC, kD, kA,   tmpReg,     tmpBuf, 10,    0xfc93a039,    21
    __I   kA, kB, kC, kD,   tmpReg,     tmpBuf,  3,    0x655b59c3,     6
    __I   kD, kA, kB, kC,   tmpReg,     tmpBuf, 12,    0x8f0ccc92,    10
    __I   kC, kD, kA, kB,   tmpReg,     tmpBuf,  5,    0xffeff47d,    15
    __I   kB, kC, kD, kA,   tmpReg,     tmpBuf, 14,    0x85845dd1,    21
    __I   kA, kB, kC, kD,   tmpReg,     tmpBuf,  7,    0x6fa87e4f,     6
    __I   kD, kA, kB, kC,   tmpReg,     tmpBuf,  0,    0xfe2ce6e0,    10
    __I   kC, kD, kA, kB,   tmpReg,     tmpBuf,  9,    0xa3014314,    15
    __I   kB, kC, kD, kA,   tmpReg,     tmpBuf,  2,    0x4e0811a1,    21
    __I   kA, kB, kC, kD,   tmpReg,     tmpBuf, 11,    0xf7537e82,     6
    __I   kD, kA, kB, kC,   tmpReg,     tmpBuf,  4,    0xbd3af235,    10
    __I   kC, kD, kA, kB,   tmpReg,     tmpBuf, 13,    0x2ad7d2bb,    15
    __I   kB, kC, kD, kA,   tmpReg,     tmpBuf,  6,    0xeb86d391,    21

    # # # # #   Done   # # # # #

    # Update registers
    adda.l   kA          , backA
    adda.l   kB          , backB
    adda.l   kC          , backC
    adda.l   kD          , backD

    subi.l   #64         , dataLen
    bhi.w    hashMore

    movem.l  backA-backD ,  -(tmpBuf)   /* Keys           */

    movem.l  (sp)+       , kA-kD/backA-backD

rts

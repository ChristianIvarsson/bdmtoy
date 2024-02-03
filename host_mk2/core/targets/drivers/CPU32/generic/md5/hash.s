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


# Regular FG
.macro _FG   _A, _B, _C, _D,   _TMP    _DAT, _IDX,    _HASH
    move.l   \_D         , \_TMP
    eor.l    \_C         , \_TMP
    and.l    \_B         , \_TMP
    eor.l    \_D         , \_TMP
    add.l    (\_IDX)*4(\_DAT), \_A
    addi.l   #\_HASH     , \_A
    add.l    \_TMP       , \_A
.endm

# FG with decrementing address index (less space)
.macro _FGmm _A, _B, _C, _D,   _TMP    _DAT, _IDX,    _HASH
    move.l   \_D         , \_TMP
    eor.l    \_C         , \_TMP
    and.l    \_B         , \_TMP
    eor.l    \_D         , \_TMP
    add.l    -(\_DAT)    , \_A
    addi.l   #\_HASH     , \_A
    add.l    \_TMP       , \_A
.endm

.macro __H   _A, _B, _C, _D,   _TMP    _DAT, _IDX,    _HASH
    move.l   \_B         , \_TMP
    eor.l    \_C         , \_TMP
    eor.l    \_D         , \_TMP
    add.l    (\_IDX)*4(\_DAT), \_A
    addi.l   #\_HASH     , \_A
    add.l    \_TMP       , \_A
.endm

.macro __I   _A, _B, _C, _D,   _TMP    _DAT, _IDX,    _HASH
    move.l   \_D         , \_TMP
    not.l    \_TMP
    or.l     \_B         , \_TMP
    eor.l    \_C         , \_TMP
    add.l    (\_IDX)*4(\_DAT), \_A
    addi.l   #\_HASH     , \_A
    add.l    \_TMP       , \_A
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


    # Restore Registers
    move.l   backA       , kA
    move.l   backB       , kB
    move.l   backC       , kC
    move.l   backD       , kD

    # VERY ugly trick since we know the first 16 hashes happens in descending order
    lea.l    64(tmpBuf)  , tmpBuf

    # # # # # STEP  0 # # # # #
    _FGmm kA, kB, kC, kD,   tmpReg,     tmpBuf, 15,    0xD76AA478
    rol.l    #0x07       , kA
    add.l    kB          , kA

    # # # # # STEP  1 # # # # #
    _FGmm kD, kA, kB, kC,   tmpReg,     tmpBuf, 14,    0xE8C7B756
    swap     kD
    ror.l    #0x04       , kD
    add.l    kA          , kD

    # # # # # STEP  2 # # # # #
    _FGmm kC, kD, kA, kB,   tmpReg,     tmpBuf, 13,    0x242070DB
    swap     kC
    rol.l    #0x01       , kC
    add.l    kD          , kC

    # # # # # STEP  3 # # # # #
    _FGmm kB, kC, kD, kA,   tmpReg,     tmpBuf, 12,    0xC1BDCEEE
    swap     kB
    rol.l    #0x06       , kB
    add.l    kC          , kB

    # # # # # STEP  4 # # # # #
    _FGmm kA, kB, kC, kD,   tmpReg,     tmpBuf, 11,    0xF57C0FAF
    rol.l    #0x07       , kA
    add.l    kB          , kA

    # # # # # STEP  5 # # # # #
    _FGmm kD, kA, kB, kC,   tmpReg,     tmpBuf, 10,    0x4787C62A
    swap     kD
    ror.l    #0x04       , kD
    add.l    kA          , kD

    # # # # # STEP  6 # # # # #
    _FGmm kC, kD, kA, kB,   tmpReg,     tmpBuf,  9,    0xA8304613
    swap     kC
    rol.l    #0x01       , kC
    add.l    kD          , kC

    # # # # # STEP  7 # # # # #
    _FGmm kB, kC, kD, kA,   tmpReg,     tmpBuf,  8,    0xFD469501
    swap     kB
    rol.l    #0x06       , kB
    add.l    kC          , kB

    # # # # # STEP  8 # # # # #
    _FGmm kA, kB, kC, kD,   tmpReg,     tmpBuf,  7,    0x698098D8
    rol.l    #0x07       , kA
    add.l    kB          , kA

    # # # # # STEP  9 # # # # #
    _FGmm kD, kA, kB, kC,   tmpReg,     tmpBuf,  6,    0x8B44F7AF
    swap     kD
    ror.l    #0x04       , kD
    add.l    kA          , kD

    # # # # # STEP 10 # # # # #
    _FGmm kC, kD, kA, kB,   tmpReg,     tmpBuf,  5,    0xFFFF5BB1
    swap     kC
    rol.l    #0x01       , kC
    add.l    kD          , kC

    # # # # # STEP 11 # # # # #
    _FGmm kB, kC, kD, kA,   tmpReg,     tmpBuf,  4,    0x895CD7BE
    swap     kB
    rol.l    #0x06       , kB
    add.l    kC          , kB

    # # # # # STEP 12 # # # # #
    _FGmm kA, kB, kC, kD,   tmpReg,     tmpBuf,  3,    0x6B901122
    rol.l    #0x07       , kA
    add.l    kB          , kA

    # # # # # STEP 13 # # # # #
    _FGmm kD, kA, kB, kC,   tmpReg,     tmpBuf,  2,    0xFD987193
    swap     kD
    ror.l    #0x04       , kD
    add.l    kA          , kD

    # # # # # STEP 14 # # # # #
    _FGmm kC, kD, kA, kB,   tmpReg,     tmpBuf,  1,    0xA679438E
    swap     kC
    rol.l    #0x01       , kC
    add.l    kD          , kC

    # # # # # STEP 15 # # # # #
    _FGmm kB, kC, kD, kA,   tmpReg,     tmpBuf,  0,    0x49B40821
    swap     kB
    rol.l    #0x06       , kB
    add.l    kC          , kB

    # # # # # STEP 16 # # # # #
    _FG   kA, kD, kB, kC,   tmpReg,     tmpBuf, 14,    0xf61e2562
    rol.l    #0x05       , kA
    add.l    kB          , kA

    # # # # # STEP 17 # # # # #
    _FG   kD, kC, kA, kB,   tmpReg,     tmpBuf,  9,    0xc040b340
    swap     kD
    ror.l    #0x07       , kD
    add.l    kA          , kD

    # # # # # STEP 18 # # # # #
    _FG   kC, kB, kD, kA,   tmpReg,     tmpBuf,  4,    0x265e5a51
    swap     kC
    ror.l    #0x02       , kC
    add.l    kD          , kC

    # # # # # STEP 19 # # # # #
    _FG   kB, kA, kC, kD,   tmpReg,     tmpBuf, 15,    0xe9b6c7aa
    swap     kB
    rol.l    #0x04       , kB
    add.l    kC          , kB

    # # # # # STEP 20 # # # # #
    _FG   kA, kD, kB, kC,   tmpReg,     tmpBuf, 10,    0xd62f105d
    rol.l    #0x05       , kA
    add.l    kB          , kA

    # # # # # STEP 21 # # # # #
    _FG   kD, kC, kA, kB,   tmpReg,     tmpBuf,  5,    0x02441453
    swap     kD
    ror.l    #0x07       , kD
    add.l    kA          , kD

    # # # # # STEP 22 # # # # #
    _FG   kC, kB, kD, kA,   tmpReg,     tmpBuf,  0,    0xd8a1e681
    swap     kC
    ror.l    #0x02       , kC
    add.l    kD          , kC

    # # # # # STEP 23 # # # # #
    _FG   kB, kA, kC, kD,   tmpReg,     tmpBuf, 11,    0xe7d3fbc8
    swap     kB
    rol.l    #0x04       , kB
    add.l    kC          , kB

    # # # # # STEP 24 # # # # #
    _FG   kA, kD, kB, kC,   tmpReg,     tmpBuf,  6,    0x21e1cde6
    rol.l    #0x05       , kA
    add.l    kB          , kA

    # # # # # STEP 25 # # # # #
    _FG   kD, kC, kA, kB,   tmpReg,     tmpBuf,  1,    0xc33707d6
    swap     kD
    ror.l    #0x07       , kD
    add.l    kA          , kD

    # # # # # STEP 26 # # # # #
    _FG   kC, kB, kD, kA,   tmpReg,     tmpBuf, 12,    0xf4d50d87
    swap     kC
    ror.l    #0x02       , kC
    add.l    kD          , kC

    # # # # # STEP 27 # # # # #
    _FG   kB, kA, kC, kD,   tmpReg,     tmpBuf,  7,    0x455a14ed
    swap     kB
    rol.l    #0x04       , kB
    add.l    kC          , kB

    # # # # # STEP 28 # # # # #
    _FG   kA, kD, kB, kC,   tmpReg,     tmpBuf,  2,    0xa9e3e905
    rol.l    #0x05       , kA
    add.l    kB          , kA

    # # # # # STEP 29 # # # # #
    _FG   kD, kC, kA, kB,   tmpReg,     tmpBuf, 13,    0xfcefa3f8
    swap     kD
    ror.l    #0x07       , kD
    add.l    kA          , kD

    # # # # # STEP 30 # # # # #
    _FG   kC, kB, kD, kA,   tmpReg,     tmpBuf,  8,    0x676f02d9
    swap     kC
    ror.l    #0x02       , kC
    add.l    kD          , kC

    # # # # # STEP 31 # # # # #
    _FG   kB, kA, kC, kD,   tmpReg,     tmpBuf,  3,    0x8d2a4c8a
    swap     kB
    rol.l    #0x04       , kB
    add.l    kC          , kB

    # # # # # STEP 32 # # # # #
    __H   kA, kB, kC, kD,   tmpReg,     tmpBuf, 10,    0xfffa3942
    rol.l    #0x04       , kA
    add.l    kB          , kA

    # # # # # STEP 33 # # # # #
    __H   kD, kA, kB, kC,   tmpReg,     tmpBuf,  7,    0x8771f681
    swap     kD
    ror.l    #0x05       , kD
    add.l    kA          , kD

    # # # # # STEP 34 # # # # #
    __H   kC, kD, kA, kB,   tmpReg,     tmpBuf,  4,    0x6d9d6122
    swap     kC
    add.l    kD          , kC

    # # # # # STEP 35 # # # # #
    __H   kB, kC, kD, kA,   tmpReg,     tmpBuf,  1,    0xfde5380c
    swap     kB
    rol.l    #0x07       , kB
    add.l    kC          , kB

    # # # # # STEP 36 # # # # #
    __H   kA, kB, kC, kD,   tmpReg,     tmpBuf, 14,    0xa4beea44
    rol.l    #0x04       , kA
    add.l    kB          , kA

    # # # # # STEP 37 # # # # #
    __H   kD, kA, kB, kC,   tmpReg,     tmpBuf, 11,    0x4bdecfa9
    swap     kD
    ror.l    #0x05       , kD
    add.l    kA          , kD

    # # # # # STEP 38 # # # # #
    __H   kC, kD, kA, kB,   tmpReg,     tmpBuf,  8,    0xf6bb4b60
    swap     kC
    add.l    kD          , kC

    # # # # # STEP 39 # # # # #
    __H   kB, kC, kD, kA,   tmpReg,     tmpBuf,  5,    0xbebfbc70
    swap     kB
    rol.l    #0x07       , kB
    add.l    kC          , kB

    # # # # # STEP 40 # # # # #
    __H   kA, kB, kC, kD,   tmpReg,     tmpBuf,  2,    0x289b7ec6
    rol.l    #0x04       , kA
    add.l    kB          , kA

    # # # # # STEP 41 # # # # #
    __H   kD, kA, kB, kC,   tmpReg,     tmpBuf, 15,    0xeaa127fa
    swap     kD
    ror.l    #0x05       , kD
    add.l    kA          , kD

    # # # # # STEP 42 # # # # #
    __H   kC, kD, kA, kB,   tmpReg,     tmpBuf, 12,    0xd4ef3085
    swap     kC
    add.l    kD          , kC

    # # # # # STEP 43 # # # # #
    __H   kB, kC, kD, kA,   tmpReg,     tmpBuf,  9,    0x04881d05
    swap     kB
    rol.l    #0x07       , kB
    add.l    kC          , kB

    # # # # # STEP 44 # # # # #
    __H   kA, kB, kC, kD,   tmpReg,     tmpBuf,  6,    0xd9d4d039
    rol.l    #0x04       , kA
    add.l    kB          , kA

    # # # # # STEP 45 # # # # #
    __H   kD, kA, kB, kC,   tmpReg,     tmpBuf,  3,    0xe6db99e5
    swap     kD
    ror.l    #0x05       , kD
    add.l    kA          , kD

    # # # # # STEP 46 # # # # #
    __H   kC, kD, kA, kB,   tmpReg,     tmpBuf,  0,    0x1fa27cf8
    swap     kC
    add.l    kD          , kC

    # # # # # STEP 47 # # # # #
    __H   kB, kC, kD, kA,   tmpReg,     tmpBuf, 13,    0xc4ac5665
    swap     kB
    rol.l    #0x07       , kB
    add.l    kC          , kB

    # # # # # STEP 48 # # # # #
    __I   kA, kB, kC, kD,   tmpReg,     tmpBuf, 15,    0xf4292244
    rol.l    #0x06       , kA
    add.l    kB          , kA

    # # # # # STEP 49 # # # # #
    __I   kD, kA, kB, kC,   tmpReg,     tmpBuf,  8,    0x432aff97
    swap     kD
    ror.l    #0x06       , kD
    add.l    kA          , kD

    # # # # # STEP 50 # # # # #
    __I   kC, kD, kA, kB,   tmpReg,     tmpBuf,  1,    0xab9423a7
    swap     kC
    ror.l    #0x01       , kC
    add.l    kD          , kC

    # # # # # STEP 51 # # # # #
    __I   kB, kC, kD, kA,   tmpReg,     tmpBuf, 10,    0xfc93a039
    swap     kB
    rol.l    #0x05       , kB
    add.l    kC          , kB

    # # # # # STEP 52 # # # # #
    __I   kA, kB, kC, kD,   tmpReg,     tmpBuf,  3,    0x655b59c3
    rol.l    #0x06       , kA
    add.l    kB          , kA

    # # # # # STEP 53 # # # # #
    __I   kD, kA, kB, kC,   tmpReg,     tmpBuf, 12,    0x8f0ccc92
    swap     kD
    ror.l    #0x06       , kD
    add.l    kA          , kD

    # # # # # STEP 54 # # # # #
    __I   kC, kD, kA, kB,   tmpReg,     tmpBuf,  5,    0xffeff47d
    swap     kC
    ror.l    #0x01       , kC
    add.l    kD          , kC

    # # # # # STEP 55 # # # # #
    __I   kB, kC, kD, kA,   tmpReg,     tmpBuf, 14,    0x85845dd1
    swap     kB
    rol.l    #0x05       , kB
    add.l    kC          , kB

    # # # # # STEP 56 # # # # #
    __I   kA, kB, kC, kD,   tmpReg,     tmpBuf,  7,    0x6fa87e4f
    rol.l    #0x06       , kA
    add.l    kB          , kA

    # # # # # STEP 57 # # # # #
    __I   kD, kA, kB, kC,   tmpReg,     tmpBuf,  0,    0xfe2ce6e0
    swap     kD
    ror.l    #0x06       , kD
    add.l    kA          , kD

    # # # # # STEP 58 # # # # #
    __I   kC, kD, kA, kB,   tmpReg,     tmpBuf,  9,    0xa3014314
    swap     kC
    ror.l    #0x01       , kC
    add.l    kD          , kC

    # # # # # STEP 59 # # # # #
    __I   kB, kC, kD, kA,   tmpReg,     tmpBuf,  2,    0x4e0811a1
    swap     kB
    rol.l    #0x05       , kB
    add.l    kC          , kB

    # # # # # STEP 60 # # # # #
    __I   kA, kB, kC, kD,   tmpReg,     tmpBuf, 11,    0xf7537e82
    rol.l    #0x06       , kA
    add.l    kB          , kA

    # # # # # STEP 61 # # # # #
    __I   kD, kA, kB, kC,   tmpReg,     tmpBuf,  4,    0xbd3af235
    swap     kD
    ror.l    #0x06       , kD
    add.l    kA          , kD

    # # # # # STEP 62 # # # # #
    __I   kC, kD, kA, kB,   tmpReg,     tmpBuf, 13,    0x2ad7d2bb
    swap     kC
    ror.l    #0x01       , kC
    add.l    kD          , kC

    # # # # # STEP 63 # # # # #
    __I   kB, kC, kD, kA,   tmpReg,     tmpBuf,  6,    0xeb86d391
    swap     kB
    rol.l    #0x05       , kB
    add.l    kC          , kB

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

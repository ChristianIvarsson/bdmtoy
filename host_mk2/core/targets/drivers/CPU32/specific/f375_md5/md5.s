.global MD5asm
.global swsr

.section .text.entry

# # # # # # # # # #
# Arguments:
# a0: Source address
# d0: Length
#
# Returned data:
# a0: Next address
# a1: Length
# a2 - a5: Key A - D


# # # # # # # # # #
# Record to beat
#
# # # # # T7
# 00:01:654
# 00:01:505
entry:
    bra.w    MD5asm
swsr_ptr: .long 0xFFFA27
swsr:
    move.l  a2       ,-(sp)
    lea.l   swsr_ptr , a2
    move.b  #0x55    ,(a2)
    move.b  #0xAA    ,(a2)
    movea.l (sp)+    , a2
rts
MD5asm:

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #  
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #  
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #  
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

# a0: Source pointer
# d0: Length

    # Simulate command from adapter
    # movea.l  #0x00000000 , a0
    # move.l   #0x00040000 , d0
    # 0x00080000
    # 0x0007ffff

    # Figure out where we are and set SP to this location + 7fc
    # ( It's designed to fit together with stack inside 2k of memory )
    lea.l    entry       , a1
    lea.l    0x7fc(a1)   , sp

    clr.l    d2
    move.b   d0          , d2    /* .. */
    andi.b   #63         , d2    /* Length & 63         */
    move.l   d0          , d3    /* Total length backup */
    lea.l    (d0, a0)    , a3    /* Store end address   */
    andi.b   #0xc0       , d0    /* AND out lower 6 bits to make it multiples of 64 */

    /* * * * * * * * * * * * *      88: Temp buffer 64 bytes   */
    lea.l    -128(sp)    , sp    /* 24: Swap buffer 64 bytes   */
    move.l   #0x10325476 ,-(sp)  /* 20: D                      */
    move.l   #0x98badcfe ,-(sp)  /* 16: C                      */
    move.l   #0xefcdab89 ,-(sp)  /* 12: B                      */
    move.l   #0x67452301 ,-(sp)  /*  8: A                      */
    move.l   d0          ,-(sp)  /*  4: Size                   */
    move.l   a0          ,-(sp)  /*  0: Source pointer         */

    and.l    d0          , d0    /* Skip if less than 64 bytes */
    beq.b    smallHash

    bsr.w    Hash

    # a0, a1 / d0, d1 are retained due to calling convention
    # a0: Current source pointer

smallHash:

    lea.l    88(sp)      , a1    /* Load buffer pointer */
    move.l   a1          , (sp)  /* Further processing happens at this address */

# Append left-over bytes
    and.w    d2          , d2
    beq.b    noRemain
    move.w   d2          , d0
    subq.l   #1, d0
doCpy:
    move.b   (a0)+       , (a1)+
    dbra     d0          , doCpy

noRemain:
# Append bit
    move.b   #0x80       , (a1)+
    addq.w   #1          , d2

# All further operations happens on 64 bytes of buffer
    moveq.l  #64         , d0
    move.l   d0          , 4(sp)

# Need 8 additional bytes for length
    cmpi.b   #57         , d2
    bcs.b    hadRoom

# Damn! No room left

    # Clear remaining bytes
    sub.w    d2          , d0
    bsr.b    memClr

    bsr.w    Hash
    clr.l    d2
    moveq.l  #64         , d0
    lea.l    88(sp)      , a1

hadRoom:
    sub.w    d2          , d0
    bsr.b    memClr

# Store length << 3 in byte-swapped order
    move.l   d3          , d1
    lsl.l    #3          , d1

    lea.l    (88+56)(sp) , a1
    moveq.l  #3          , d0
swapMore:
    move.b   d1          , (a1)+
    lsr.l    #8          , d1
    dbra     d0          , swapMore

# One final hash
    bsr.w    Hash



# Dump keys etc into registers a0 to a5
    move.l   a3          , 0(sp) /* Store next address */
    move.l   d3          , 4(sp) /* Store count        */
    movem.l  (sp)+       , a0-a5
    lea.l    -24(sp)     , sp

bgnd

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #  
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #  
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #  
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #   

# dbra decrements to and checks for -1
# Hence these weird jumps

# d0 length, a1 address
doClr:
    clr.b    (a1)+
memClr:
    dbra     d0          , doClr
rts

.section .text.entry
.global detectEntry

.include "macro.inc"

# # # # # # # # # # # # # # # # # #
# Arguments:
# a0 - Base of flash
#
# Returns:
# d0: Status - You should see 1 for ok. Anything else means unable to identify flash
# d4: Flash ID           (Ignore upper 16 bits)
# d5: Extended flash ID  (Ignore upper 16 bits)
#
#

detectEntry:

    # Set stack to be 2044 bytes above entry of this code
    lea.l    detectEntry  , a1
    lea.l    0x7fc(a1)   , sp

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Check toggle flash

# Virtually all commands happens at these two addresses so let's make them global
    lea.l    0xAAAA(fBase), addrA
    lea.l    0x5554(fBase), addrB
    move.l   addrA       , cmdA
    move.w   #0x5555     , cmdB

    clr.l    flMID
    clr.l    flPID

    # Try modern flash first since it's the most likely
    bsr.b    ngSequence

    # Transform and compare MID / DID. Noret if successful
    bsr.b    checkData

    # Try oldschool method
    bsr.b    ogSequence
    bsr.b    checkData

# Failed one way or another
bdmFail:
    clr.l    retReg
    bgnd

bdmOK:
    moveq.l  #1          , retReg
    bgnd

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Utils

# # # # # # # # # # # # # # # # # # # # # # #
# Transform and check IDs
checkData:

    bsr.b    checkIds
    beq.b    bdmOK

# Figure out if connected to 2x8 or 1x16

# With AMD as an example:
# 1 x 8-bit would return  01ID
# 2 x 8-bit would return  0101 IDID
# 16-bit would return     0001 EXID

    # Shift down MSB and check if it's the same
    move.w   flMID       , d0
    lsr.w    #8          , d0
    cmp.b    d0          , flMID
    bne.b    _not2x8

    move.w   flPID       , d1
    lsr.w    #8          , d1
    cmp.b    d1          , flPID
    bne.b    _not2x8
    move.w   d0          , flMID
    move.w   d1          , flPID
_not2x8:

    bsr.b    checkIds
    beq.b    bdmOK
    rts

# # # # # # # # # # # # # # # # # # # # # # #
# Generic delay
genDelay:
    move.w   #0x1800     , d0
Dloop:
    dbra     d0          , Dloop
rts

# # # # # # # # # # # # # # # # # # # # # # #
# Query modern flash
ngSequence:

# Send ID command
    lwCMD    0x9090
    bsr.b    genDelay

# Read ID and EID
    move.w   (fBase)     , flMID
    move.w   2(fBase)    , flPID

# Send reset command - Use both methods since some ogflash will respond to this request too
# Note: LW flash uses f0 to reset while HW requires two writes of ff
    lwCMD    0xF0F0
    move.w   #0xffff     , (fBase)
    move.w   #0xffff     , (fBase)

    bra.b    sequenceDelay

# # # # # # # # # # # # # # # # # # # # # # #
# Query oldschool flash
ogSequence:

# Send ID command
    move.w   #0x9090     , (fBase)
    bsr.b    genDelay

# Read ID and EID
    move.w   (fBase)     , flMID
    move.w   2(fBase)    , flPID

# Send reset command
    move.w   #0xffff     , (fBase)
    move.w   #0xffff     , (fBase)

sequenceDelay:
    moveq.l  #4          , d1
repDelay:
    bsr.w    genDelay
    dbra     d1          , repDelay
    rts

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Perform checks

checkIds:

    lea.l    idList      , a1
    clr.l    d0

nextPart:
    move.w   (a1)+       , d0      /* Count */
    beq.b    idFail
    cmp.w    (a1)+       , flMID   /* Manufacturer */
    bne.b    skipCollection

    subq.l   #1          , d0

nextId:
    cmp.w    (a1)+       , flPID   /* Part ID */

    beq.b    idFound
    dbra     d0          , nextId
    bra.b    nextPart

skipCollection:
    lsl.l    #1          , d0      /* From count to size */
    adda.l   d0          , a1
    bra.b    nextPart

idFail:
    moveq.l  #1          , d0
    rts
idFound:
    moveq.l  #0          , d0
    rts

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# ID list

idList:
# # # # # # # # # # # # # # # # #
# AMD
.word   amdLen               /* Count */
.word   0x0001               /* Manufacturer */
amdList:
.word   0x2258               /* AM29F800BB   1024K  16-bit  Toggle   */
.word   0x22D6               /* AM29F800BT   1024K  16-bit  Toggle   */
.word   0x22AB               /* AM29F400BB   512k   16-bit  Toggle   */
.word   0x2223               /* AM29F400BT   512k   16-bit  Toggle   */
.word   0x00A7               /* AM28F010     128k    8-bit  OgStyle  */
.equ  amdLen, ((. - amdList) / 2)

# # # # # # # # # # # # # # # # #
# Atmel
.word   atmelLen             /* Count */
.word   0x001F               /* Manufacturer */
atmelList:
.word   0x005D               /* AT29C512    64K  8-bit  Paged        */
.word   0x00D5               /* AT29C010   128K  8-bit  Paged        */
.word   0x00DA               /* AT29C020   256k  8-bit  Paged        */
.equ  atmelLen, ((. - atmelList) / 2)

# # # # # # # # # # # # # # # # #
# Signal end of list
.word   0x0000               /* Count */

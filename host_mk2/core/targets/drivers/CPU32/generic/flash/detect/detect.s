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
# d6: Size of flash
# d7: Type of flash
#                   1 - 28f series flash
#                   2 - Toggle type. 29, 39 etc
#
#

detectEntry:

    # Set stack to be 2044 bytes above entry of this code
    lea.l    detectEntry  , a1
    lea.l    0x7fc(a1)   , sp

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Check toggle flash

    # Set type to toggle flash
    moveq.l  #2          , flsTyp

# Virtually all commands happens at these two addresses so let's make them global
    lea.l    0xAAAA(fBase), addrA
    lea.l    0x5554(fBase), addrB
    move.l   addrA       , cmdA
    move.w   #0x5555     , cmdB

# Send ID command
    lwCMD    0x9090
    bsr.w    genDelay

# Read ID and EID
    clr.l    flMID
    clr.l    flPID
    move.w   (fBase)     , flMID
    move.w   2(fBase)    , flPID

# Send reset command
# Note: LW flash uses f0 to reset while HW requires two writes of ff
    lwCMD    0xF0F0
    moveq.l  #4          , d1
repDelay:
    bsr.w    genDelay
    dbra     d1          , repDelay

# Default to 1 meg
    moveq.l  #0x10       , flLen
    swap     flLen


# Figure out if connected to 2x8 or 1x16

# With AMD as an example:
# 1 x 8-bit would return  01ID
# 2 x 8-bit would return  0101 IDID
# 16-bit would return     0001 EXID

    # Shift down MSB and check if it's the same
    # move.w   flMID       , d1
    # lsr.w    #8          , d1
    # cmp.b    d1          , flMID
    # beq.b    flash2x8

_1x16flash:

    # Check manufacturer IDs
    cmpi.w   #1          , flMID
    bne.b    notAMD16

    # Check AMD 1M parts
    lea.l    am1Mx16     , a1
    moveq.l  #am1Mx16ln  , d0
    bsr.b    checkSixteen
    beq.w    returnOk

    # Decrease to 512k and go over 512 x 16 parts from AMD
    lsr.l    #1          , flLen
    lea.l    am512x16    , a1
    moveq.l  #am512x16ln , d0
    bsr.b    checkSixteen
    beq.w    returnOk
    # bra.b    returnFail

# Both fail for now
notAMD16:
flash2x8:
    # bra.b    returnFail

# Failed one way or another
returnFail:
    clr.l    retReg
bgnd

returnOk:
    moveq.l  #1          , retReg
bgnd


# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Utils

genDelay:
    move.w   #0x1800     , d0
Dloop:
    dbra     d0          , Dloop
rts

# d0: Count
# a1: Array
# d5: pID to compare against
doCheckSixteen:
    cmp.w    (a1)+       , flPID
    beq.b    checkReturnOk
checkSixteen:
    dbra     d0          , doCheckSixteen
    rts

# d0: Count
# a1: Array
# d5: pID to compare against
doCheckEight:
    cmp.b    (a1)+       , flPID
    beq.b    checkReturnOk
checkEight:
    dbra     d0          , doCheckEight
    rts

checkReturnOk:
    clr.l    d0
    rts

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# ID list - AMD

# 1M x 16 bits
am1Mx16:
.word 0x2258        /* AM28F800BB */
.word 0x22D6        /* AM28F800BT */
.equ  am1Mx16ln, ((. - am1Mx16) / 2)

# 512k x 16 bits
am512x16:
.word 0x22AB        /* AM28F400BB */
.word 0x2223        /* AM28F400BT */
.equ  am512x16ln, ((. - am512x16) / 2)

.align 2

.global flashEntry
.global genDelay

.include "macro.inc"

.section .text.entry

# # # # # # # # # # # # # # # # # #
# Overall
# Arguments:
# d0 - mode.
#     3 - Bulk erase
#     2 - Selective erase
#     1 - Write
#     0 - nop
#


#   5        14          26        36
#   move.b   d0          , d2      /* .. */

flashEntry:

    # Set stack to be 1020 bytes above entry of this code
    # Do this every time to allow manual running
    lea.l    flashEntry  , a1
    lea.l    0x3fc(a1)   , sp

    # We want write to be the fastest
    cmpi.b   #1          , opReg
    bne.b    notWrite

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Is write
    movea.l  srcAddr     , srcBck
    move.l   wrLen       , lenBck

    cmpi.b   #2          , flsTyp
    beq.w    toggleWrite


notWrite:

    # Preload register for ff compare
    moveq.l  #-1         , ffReg

    cmpi.b   #2          , opReg
    bne.b    notSelective

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Is sector erase

    cmpi.b   #2          , flsTyp
    beq.w    toggleSectorErase


notSelective:
    cmpi.b   #3          , opReg
    bne.b    notBulk

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Is bulk erase
    cmpi.b   #2          , flsTyp
    beq.w    toggleBulkErase

notBulk:

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# No other options, go to fail

# Failed one way or another
returnFail:
    clr.l    retReg
bgnd

returnOk:
    moveq.l  #1          , retReg
bgnd


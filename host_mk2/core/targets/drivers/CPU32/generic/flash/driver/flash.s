.global flashEntry
.global genDelay

.include "macro.inc"

.section .text.entry

# # # # # # # # # # # # # # # # # #
# Arguments:
#
# d7 - Flash type
#      1 - Oldschool 28f
#      2 - Toggle flash (29f, 39f etc)
#      3 - Likely atmel eeprom class chips
#
# d0 - mode.
#      4 - Init
#      3 - Bulk erase
#      2 - Sector erase
#      1 - Write
#      0 - nop
#
#
# Init: ( 4 )
#      d0: Set to 4
#      a0: Base address of flash
#      d7: Read above
#
# Bulk erase: (3)
#      a0: Erase from
#      a1: Erase up to
#      d7: Read above
#
# Sector erase: (2)     (Flash chips that only support bulk will do that instead)
#      a0: Erase from
#      a1: Erase up to
#      d7: Read above
#
# Write: (1)
#      a0: Destination address, will autoincrement
#      a1: Source buffer address, You only have to set this once
#      d1: Number of bytes to write. You only have to set this once or when a new length is required
#      d7: Read above
#

flashEntry:

    # We want write to be the fastest
    cmpi.b   #1          , opReg
    bne.b    checkSectErase

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Is write
    movea.l  writeSrc    , wrkSrc
    move.l   writeLen    , wrkLen

doWrite:
    bra.w    bdmFail

checkSectErase:

    # Preload register for ff compare
    moveq.l  #-1         , ffReg

    cmpi.b   #2          , opReg
    bne.b    checkBulkErase

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Is sector erase
doSector:
    bra.w    bdmFail

checkBulkErase:
    cmpi.b   #3          , opReg
    bne.b    checkInit

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Is bulk erase
doBulk:
    bra.w    bdmFail

checkInit:
    cmpi.b   #4          , opReg
    bne.b    bdmFail

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Is init

    # Set stack to be 1020 bytes above entry of this code
    lea.l    flashEntry  , a1
    lea.l    0x3fc(a1)   , sp

# OG flash
    cmpi.b   #1          , flashType
    bne.b    checkToggle
    lea.l    ogOffsets   , a3
    bsr.b    installJump
    bra.b    bdmOK

# Toggle flash
checkToggle:
    cmpi.b   #2          , flashType
    bne.b    bdmFail

    lea.l    toggleOffs  , a3
    bsr.b    installJump
    bra.w    toggleInit







# Failed one way or another
bdmFail:
    clr.l    retReg
    bgnd

bdmOK:
    moveq.l  #1          , retReg
    bgnd


# # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Install pointers
installJump:
    lea.l    flashEntry  , a1
    lea.l    mainOffs    , a2

    moveq.l  #2          , d0
ptrLoop:
    move.w   (a2)+       , d2
    move.w   (a3)+       , (d2.w, a1)
    dbra     d0          , ptrLoop
    rts

# write, sector erase, bulk erase
mainOffs:   .word doWrite+2              , doSector+2                    , doBulk+2
ogOffsets:  .word (ogWrite-doWrite)-2    , (ogErase-doSector)-2          , (ogErase-doBulk)-2
toggleOffs: .word (toggleWrite-doWrite)-2, (toggleSectorErase-doSector)-2, (toggleBulkErase-doBulk)-2

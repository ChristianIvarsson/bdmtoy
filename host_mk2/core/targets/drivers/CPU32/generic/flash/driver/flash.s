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
#

flashEntry:

    # We want write to be the fastest
    cmpi.b   #1          , opReg
    bne.b    notWrite

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Is write
    movea.l  writeSrc    , wrkSrc
    move.l   writeLen    , wrkLen

    cmpi.b   #2          , flashType
    beq.w    toggleWrite


notWrite:

    # Preload register for ff compare
    moveq.l  #-1         , ffReg

    cmpi.b   #2          , opReg
    bne.b    notSelective

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Is sector erase
    cmpi.b   #2          , flashType
    beq.w    toggleSectorErase


notSelective:
    cmpi.b   #3          , opReg
    bne.b    notBulk

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Is bulk erase
    cmpi.b   #2          , flashType
    beq.w    toggleBulkErase

notBulk:
    cmpi.b   #4          , opReg
    bne.b    notInit

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Is init

    # Set stack to be 1020 bytes above entry of this code
    lea.l    flashEntry  , spTemp
    lea.l    0x3fc(spTemp), sp

    cmpi.b   #2          , flashType
    beq.w    toggleInit

notInit:

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# No other options, go to fail

# Failed one way or another
returnFail:
    clr.l    retReg
bgnd

returnOk:
    moveq.l  #1          , retReg
bgnd


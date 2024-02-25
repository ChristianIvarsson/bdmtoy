
.global entry
.global regData
.global writeData
.global eraseData

.section .init

# Couple of notes etc

# While this driver should be able to flash parts with two modules of CMFI, it won't be entirely straightforward:
# 1. It's only fetching one bank of addresses so you have to update them accordingly between modules
# 2. Bulk erase will _NOT WORK ON MODULE 2!_

# Init ( 4 - You must run this once before doing anything else )
# 1. it's using self-modifying code just to prevent you from doing anything before this has been run
# - d0 : 4
# - no arguments -

# Writing ( 1 )
# 1. Writes must be aligned to 64-byte boundaries and lengths must be in multiples of said number
# 2. If you are to write shadow, it expects you to write it from offset 04_0000 and up - this applies to module 2 too!
# - - Driver does some transparent conversion behind the scenes and writes to shadow space starting from 0
# - d0 : 1
# - d1 : < Number of words (16-bit) to write >
# - a0 : < Destination offset inside the array >  ( Will auto-increment after each _SUCCESSFUL_ write. Otherwise stay the same )
# - a1 : < Data buffer location >

# Erasing, sector ( 2 )
# 1. Sector erase is using the sane order where sector 0 is bit 0, sector 1 bit 1 etc. Not the weirdo MSb == sector 0 as Motorola's weird-ass driver does it
# - d0 : 2
# - d1 : < sector bitmask >

# Erasing, bulk ( 3 )
# - d0 : 3
# - no arguments -

# All commands
# Returned status is in d0.
# 1 - All went ok - Any other value means something went wrong. Check cmfi.h enum enStatus

entry:                             /* Offset from label - size     */
    bra.w    cmfiEntry             /*  0 - 4 - Entry jump          */

# Offset 4
.align 4
regData:
.long  0x000000                    /*  0 - 4 - CMFI base           */
.long  0xFFF800                    /*  4 - 4 - CMFIMCR             */
.long  0xFFF804                    /*  8 - 4 - CMFITST             */
.long  0xFFF80C                    /* 12 - 4 - CMFICTL1 / CMFICTL2 */
.long  0xFFFA27                    /* 16 - 4 - SWSR                */

# Default data stored in driver is V5.1 running at 24 MHz ( Trionic 8's MCP )

# Offset 24
# Write data ( Size 84 )
writeData:
.long  0x22020000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF /*  0 - 36 - ctlProg          */
.word  0x0000    , 0xFFFF    , 0xFFFF    , 0xFFFF    , 0xFFFF    , 0xFFFF    , 0xFFFF    , 0xFFFF    , 0xFFFF     /* 36 - 18 - pawsProgData     */
.word  0xFFFF    , 0xFFFF    , 0xFFFF    , 0xFFFF    , 0xFFFF    , 0xFFFF    , 0xFFFF    , 0xFFFF    , 0xFFFF     /* 54 - 18 - pawsProgPulses   */
.byte  0x00      , 0x00      , 0x00      , 0x00      , 0x00      , 0x00      , 0x00      , 0x00      , 0x00       /* 72 -  9 - pawsProgMode     */
.byte  0x00                                                                                                       /* 81 -  1 - Reserved         */
.word  10000                                                                                                      /* 82 -  2 - maxProgramPulses */

# Offset 108
# Erase data ( Size 84 )
eraseData:
.long  0x21790000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF /*  0 - 36 - ctlErase         */
.word  0x0000    , 0xFFFF    , 0xFFFF    , 0xFFFF    , 0xFFFF    , 0xFFFF    , 0xFFFF    , 0xFFFF    , 0xFFFF     /* 36 - 18 - pawsEraseData    */
.word  0xFFFF    , 0xFFFF    , 0xFFFF    , 0xFFFF    , 0xFFFF    , 0xFFFF    , 0xFFFF    , 0xFFFF    , 0xFFFF     /* 54 - 18 - pawsErasePulses  */
.byte  0x00      , 0x00      , 0x00      , 0x00      , 0x00      , 0x00      , 0x00      , 0x00      , 0x00       /* 72 -  9 - pawsEraseMode    */
.byte  0x00                                                                                                       /* 81 -  1 - Reserved         */
.word  1                                                                                                          /* 82 -  2 - maxErasePulses   */

cmfiEntry:

    cmpi.b   #1          , d0
    bne.b    checkSectEr

doWrite:
    bra.w    failJump

checkSectEr:
    cmpi.b   #2          , d0
    bne.b    checkBulk

doSector:
    bra.w    failJump

checkBulk:
    cmpi.b   #3          , d0
    bne.b    checkInit

doBulk:
    bra.w    failJump

checkInit:
    cmpi.b   #4          , d0
    bne.b    failJump

# Init

    # Set sp to entrypoint + 7fc
    lea.l    entry       , a1
    lea.l    0x7fc(a1)   , sp

    bsr.w    serviceWatchdog
    bsr.b    installJump

# Enter bdm mode
    moveq.l  #1          , d0
    bra.b    bdmJump

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# ...
failJump:
    moveq.l  #0          , d0

bdmJump:
    bgnd
    bra.b    bdmJump

# # # # # # # #
# Write
#
# Arguments
# d0 - 1 < write >
# a0 - Destination offset inside CMFI
# a1 - Source buffer
# d1 - Length - In words!
#
# Returns
# d0 - status ( 1 = ok, others pending )
asmWrite:
    movem.l  d1/a0-a1    , -(sp)
    # 8: Source
    # 4: Destination
    # 0: nWords
    bsr.w    write
    movem.l  (sp)+       , d1/a0-a1

# Do not update destination address if driver failed
    cmpi.w   #1          , d0
    bne.b    bdmJump

# Convert nWords to nBytes and add it to destination address
    clr.l    d2
    move.w   d1          , d2
    lsl.l    #1          , d2
    adda.l   d2          , a0
    bra.b    bdmJump

# # # # # # # #
# Sector erase
#
# Arguments
# d0 - 2 < sector erase >
# d1 - sector bitfield    ( Bit 0 - partition 0, bit 1 - partition 1 etc )
#
# Returns
# d0 - status ( 1 = ok, others pending )
asmSector:
    move.l   d1          ,-(sp)
    bsr.w    sectorErase
    addq.l   #4          , sp
    bra.b    bdmJump

# # # # # # # #
# Bulk erase
#
# Arguments
# d0 - 3 < bulk erase >
#
# Returns
# d0 - status ( 1 = ok, others pending )
asmBulk:
    bsr.w    bulkErase
    bra.b    bdmJump

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Install pointers
installJump:
    lea.l    entry       , a1
    lea.l    mainOffs    , a2
    lea.l    cmfiOffs    , a3

    moveq.l  #2          , d0
ptrLoop:
    move.w   (a2)+       , d2
    move.w   (a3)+       , (d2.w, a1)
    dbra     d0          , ptrLoop
    rts

mainOffs:  .word (doWrite-entry)+2    , (doSector-entry)+2     , (doBulk-entry)+2
cmfiOffs:  .word (asmWrite-doWrite)-2 , (asmSector-doSector)-2 , (asmBulk-doBulk)-2

.global toggleSectorErase
.global toggleBulkErase
.global toggleWrite
.global toggleInit

.include "macro.inc"


# # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# a0 - Start from
# a1 - Up to
# Entry - toggleSectorErase

sendSectErase:
    lwCMD    0x8080
    move.w   cmdA        , (addrA)
    move.w   cmdB        , (addrB)
    move.w   #0x3030     , (eraseAddr)

sectorWait:
    move.w   (eraseAddr) , tmpRegA
    cmp.w    (eraseAddr) , tmpRegA
    bne.b    sectorWait

toggleSectorErase:
    cmp.w    (eraseAddr) , ffReg
    bne.b    sendSectErase
    addq.l   #2          , eraseAddr
    cmpa.l   endAddr     , eraseAddr
    bcs.b    toggleSectorErase
    bra.b    toggleOk

# # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# a0 - Start from
# a1 - Up to
# Entry - toggleBulkErase

sendBulkErase:
    lwCMD    0x8080
    lwCMD    0x1010

bulkWait:
    move.w   (eraseAddr) , tmpRegA
    cmp.w    (eraseAddr) , tmpRegA
    bne.b    bulkWait

toggleBulkErase:
    cmp.w    (eraseAddr) , ffReg
    bne.b    sendBulkErase
    addq.l   #2          , eraseAddr
    cmpa.l   endAddr     , eraseAddr
    bcs.b    toggleBulkErase
toggleOk:
    moveq.l  #1          , retReg

writeDone:
    bgnd

# # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# a0 - Dst
# a1 - Src
# d1 - Length, In words!
# Entry - toggleWrite

writeNext:
    subq.w   #1          , wrkLen
    beq.b    writeDone
toggleWrite:
    cmpm.w   (writeDst)+ , (wrkSrc)+
    beq.b    writeNext
    lwCMD    0xA0A0

    move.w   -(wrkSrc)   , -(writeDst)

writeWait:
    move.w   (writeDst)  , tmpRegA
    cmp.w    (writeDst)  , tmpRegA
    bne.b    writeWait
    bra.b    toggleWrite

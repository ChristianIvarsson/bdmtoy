.global toggleSectorErase
.global toggleBulkErase
.global toggleWrite

.include "macro.inc"

# # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# a0 - Start from
# a1 - Up to
# Entry - toggleSectorErase

sendSectErase:
    lwCMD    0x8080
    move.w   cmdA        , (addrA)
    move.w   cmdB        , (addrB)
    move.w   #0x3030     , (dstAddr)

sectorWait:
    move.w   (dstAddr)   , tmpRegA
    cmp.w    (dstAddr)   , tmpRegA
    bne.b    sectorWait

toggleSectorErase:
    cmp.w    (dstAddr)   , ffReg
    bne.b    sendSectErase
    addq.l   #2          , dstAddr
    cmpa.l   endAddr     , dstAddr
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
    move.w   (dstAddr)   , tmpRegA
    cmp.w    (dstAddr)   , tmpRegA
    bne.b    bulkWait

toggleBulkErase:
    cmp.w    (dstAddr)   , ffReg
    bne.b    sendBulkErase
    addq.l   #2          , dstAddr
    cmpa.l   endAddr     , dstAddr
    bcs.b    toggleBulkErase
toggleOk:
    moveq.l  #1          , retReg
    bgnd

# # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# a0 - Dst
# a1 - Src
# d1 - Length, In words!
# Entry - toggleWrite

writeNext:
    subq.w   #1          , wrLen
    beq.b    writeDone
toggleWrite:
    cmpm.w   (dstAddr)+  , (srcAddr)+
    beq.b    writeNext
    lwCMD    0xA0A0

    move.w   -(srcAddr)  , -(dstAddr)

writeWait:
    move.w   (dstAddr)   , tmpRegA
    cmp.w    (dstAddr)   , tmpRegA
    bne.b    writeWait
    bra.b    toggleWrite

writeDone:
    movea.l  srcBck      , srcAddr
    move.l   lenBck      , wrLen
    bgnd

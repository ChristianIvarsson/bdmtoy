.global ogWrite
.global ogErase

.include "macro.inc"


.set tmpRegB  , d5

ogWrite:

    # Reset retry counter
    moveq.l  #24         , tmpRegA

writeCompare:
    # Is it even necessary to write this byte?
    cmpm.w   (writeDst)+ , (wrkSrc)+
    beq.b    decDataCounter

writePulse:
    # Send write command
    move.w   #0x4040     ,-(writeDst) 
    move.w   -(wrkSrc)   , (writeDst) 
    bsr.b    Delay_10uS

    # Send "Write compare" command
    move.w   #0xC0C0     , (writeDst)
    bsr.b    Delay_6uS

    # Did it stick?
    cmpm.w   (writeDst)+ , (wrkSrc)+
    bne.b    decTryCounter
    # Paranoia; Revert back to read-mode and compare once more. 
    clr.w    -(writeDst)
    tst.w    -(wrkSrc)
    bra.b    writeCompare

decTryCounter:
    # Nope. Decrement tries and try again.. if allowed
    dbra     tmpRegA     , writePulse
    moveq.l  #0          , retReg
    bgnd

decDataCounter:
    subq.w   #1          , wrkLen
    bne.b    ogWrite

    # Workaround for CAT28 errata
    clr.w    (writeDst)
    bgnd

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

Delay_10uS:
    moveq.l #15      , tmpRegB
us10loop:
    subq.l  #1       , tmpRegB
    bne.b   us10loop
rts

Delay_6uS:
    moveq.l #8       , tmpRegB
us6loop:
    subq.l  #1       , tmpRegB
    bne.b   us6loop
rts

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

ogErase:

    # Zero flash
    # Make a copy of start addr
    movea.l  eraseAddr   , wrkSrc

zeroResetTries:
    moveq.l  #25         , tmpRegA

zeroCheck:
    tst.w    (wrkSrc)+
    beq.b    zeroCheckAddress

zeroPulse:
    move.w   #0x4040     ,-(wrkSrc)
    clr.w    (wrkSrc)
    bsr.b    Delay_10uS
    move.w   #0xC0C0     , (wrkSrc)
    bsr.b    Delay_6uS
    tst.w    (wrkSrc)+
    bne.b    zeroDecTryCounter
    clr.w    -(wrkSrc)
    bra.b    zeroCheck

zeroDecTryCounter:
    subq.b   #1          , tmpRegA 
    beq.b    EndFF
    bra.b    zeroPulse
zeroCheckAddress:
    cmpa.l   endAddr     , wrkSrc 
    bcs.b    zeroResetTries


    # Format flash

    # Maximum number of tries
    move.w   #1000       , d0
    move.w   #0x2020     , d1

FFloop:
    cmp.w    (eraseAddr)+, ffReg
    beq.b    DataisFF
    move.w   d1          ,-(eraseAddr)
    move.w   d1          , (eraseAddr)

    # Wait for 10~ mS (10,05 ish)
    move.w   #0x4240     , tmpRegA
mSloop:
    dbra     tmpRegA     , mSloop

    move.w   #0xA0A0     , (eraseAddr)
    bsr.b    Delay_6uS
    cmp.w    (eraseAddr) , ffReg
    bne.b    DecFF
    clr.w    (eraseAddr)
    bra.b    FFloop
DecFF:
    subq.w   #1          , d0
    beq.b    EndFF
DataisFF:
    cmpa.l   endAddr     , eraseAddr
    bcs.b    FFloop

    moveq.l  #1          , d0
EndFF:
    bgnd

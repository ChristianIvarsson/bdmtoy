.global pagedErase
.global pagedWrite128
.global pagedWrite256

.include "macro.inc"


# It's all an illusion. Say "OK" to the host and let the pagewriter take care of it
# # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# a0 - Start from
# a1 - Up to
pagedErase:
    moveq.l  #-1         , d0
eraseWait:
    nop
    dbra     d0          , eraseWait
    moveq.l  #1          , d0
    bgnd


# # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# a0 - Dst
# a1 - Src
# d1 - Length, In words!

pagedWrite256:
    # Each page has 256 bytes of data. 
    moveq.l  #127        , d7
    bra.b    prepPage

pagedWrite128:
    # Each page has 128 bytes of data. 
    moveq.l  #63         , d7

prepPage:

    # Make it dwords!
    lsr.l    #1          , wrkLen

beginPage:
    movea.l  wrkSrc      , a5      /* Make a copy of source address        */
    movea.l  a0          , a6      /* Make a copy of destination address   */

nextPage:
    move.w   d7          , tmpRegA /* Make a copy of count                 */

comparePage:
    cmpm.l   (a5)+       , (a6)+
    bne.b    doWrite
    dbra     tmpRegA     , comparePage

# Data is identical
    movea.l  a5          , wrkSrc  /* Update original source pointer       */
    movea.l  a6          , a0      /* Update original destination pointer  */

    sub.w    d7          , wrkLen
    subq.w   #1          , wrkLen
    bhi.b    nextPage
    bgnd


# Data is different
doWrite:
    movea.l  wrkSrc      , a5      /* Make a copy of source address        */
    movea.l  a0          , a6      /* Make a copy of destination address   */
    move.w   d7          , tmpRegA /* Make a copy of count                 */

# Unlock
    lwCMD    0xA0A0

writeLoop:
    move.l   (a5)+       , (a6)+
    dbra     tmpRegA     , writeLoop

busyWait:
    move.w   (a0)        , tmpRegA
    cmp.w    (a0)        , tmpRegA
    bne.b    busyWait
    bra.b    beginPage

.global pagedErase
.global pagedWrite128
.global pagedWrite256

.include "macro.inc"


# It's all an illusion. Say "OK" to the host and let the pagewiter take care of it
# # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# a0 - Start from
# a1 - Up to
pagedErase:
    # bsr.w    Delay
    moveq.l  #1          , d0
    bgnd


# # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# a0 - Dst
# a1 - Src
# d1 - Length, In words!

pagedWrite256:
    # Each page has 256 bytes of data. 
    move.w   #255        , d7
    bra.b    beginPage

pagedWrite128:
    # Each page has 128 bytes of data. 
    moveq.l  #127        , d7



beginPage:
    movea.l  wrkSrc      , a5      /* Make a copy of source address        */
    movea.l  a0          , a6      /* Make a copy of destination address   */

nextPage:
    move.w   d7          , tmpRegA /* Make a copy of count                 */

comparePage:
    cmpm.w   (a5)+       , (a6)+
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
    move.w   (a5)+       , (a6)+
    dbra     tmpRegA     , writeLoop

busyWait:
    move.w   (a0)        , tmpRegA
    cmp.w    (a0)        , tmpRegA
    bne.b    busyWait
    bra.b    beginPage

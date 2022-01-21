.thumb
.global TAP_PreciseDelay

# Takes:
# 22~ cycles only to jump to this function, do the check and then leave
# 6-cycle granularity of the check-loop
TAP_PreciseDelay:
    ldr    r3, =DWTADDR
    ldr    r3, [r3]
    ldr    r2, [r3]
TAP_PreciseIn:
    ldr    r1, [r3]
    sub    r1,  r2
    cmp    r1,  r0
    bmi TAP_PreciseIn
bx lr

DWTADDR: .long 0xE0001004

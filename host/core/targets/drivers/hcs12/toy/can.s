

# 0x180
# x: CAN base
.align 2
initCAN:

/*  
    pshy

# Configure pin mapping etc
    ldd #0
    ldy #0x0250

    ldaa #0x05
    staa 0,y

    ldaa #0x2a
    staa 2,y

    ldaa #0x00
    staa 3,y

    ldaa #0xe0
    staa 4,y

    ldaa #0xff
    staa 5,y

    ldaa #0x00
    staa 6,y

    ldaa #0x00
    staa 7,y

    puly
*/

    # Enable controller, in case it's off. CANCTL1
    ldaa     1,x
    oraa    #0x80
    staa     1,x

    # Enter initialization mode. CANCTL0
    ldaa     0,x
    oraa     #1
    staa     0,x

    # Poll INITAK. CANCTL1
CANwaitoff:
    ldaa     1,x
    anda     #1
    beq CANwaitoff

    # Set WUPM to 1, no test mode, no listen-only mode, clock source: ext osc
    ldaa    #0x84
    staa     1,x


# Set baudrate
    # (4194000 / 4) / (1 + TSEG1 + TSEG2): 47.659

    # BTR0. Divide: 4, SJW: 4
    ldaa    #0xC3
    staa     2,x

    # BTR1. TSEG1: 16, TSEG2: 5, SAMP: take three samples
    ldaa    #0xCF
    staa     3,x


# Mess with filters
    # canidac.
    ldaa    #0x10
    staa    11,x


    ldaa    #0x20
    # idar 0
    staa    16,x

    ldaa    #0
    # idar 1
    staa    17,x
    # idar 2
    staa    18,x
    # idar 3
    staa    19,x
    # idar 4
    staa    24,x
    # idar 5
    staa    25,x
    # idar 6
    staa    26,x
    # idar 7
    staa    27,x

    # idmr 0
    staa    20,x
    # idmr 2
    staa    22,x
    # idmr 4
    staa    28,x
    # idmr 6
    staa    30,x


    ldaa    #0x07
    # idmr 1
    staa    21,x
    # idmr 3
    staa    23,x
    # idmr 5
    staa    29,x
    # idmr 7
    staa    31,x


    # Leave initialization mode. CANCTL0
    ldaa     0,x
    anda     #0xFE
    staa     0,x

    # Poll INITAK. CANCTL1
CANwaiton:
    ldaa     1,x
    anda     #1
    bne CANwaiton
rts


# Y: Data ptr
# D: message box
.align 2
CANsend:

    pshx
    pshy

    # Base of controller 
    ldx #0x0180

    # CAN1TFLG
    ldaa     6,x
    beq noFree

    # Set first box
    # CAN1TBSEL
    staa    10,x

    # Store box indicated by the controller
    ldaa    10,x
    psha

    # CAN1
    leax  0x30,x

    # ID high byte << 5
    ldd      #0
    std      2,x
    ldaa    #0x20
    std      0,x

    ldab     #8

    # Store length
    stab    12,x

sendcpyl:
    ldaa     0,y
    staa     4,x
    leay     1,y
    leax     1,x
    decb
    bne sendcpyl

    # Store priority. (0, highest)
    clr      5,x

    pulb
    stab   -50,x

waitSend:
    ldab   -50,x
    andb     0,sp
    beq waitSend

    puly
    pulx
rts

noFree:
    ldd #0xfefe
    bgnd
rts

.align 2
# X: no loops
# This loop has three P + one O cycles
delayCycles:
    dex
    bne delayCycles
rts


/*
# Check one sector (512 bytes) against data to be written / has been written
# In:
#  X: Start buffer
#  D: Start flash
# Out:
#  a: 0 = identical, 1 = not ident
.align 2

verEmpty:

    pshy

    ldy #0x8000

    ldaa #0
    psha

moreEmpty:
    ldd      0,y
    cpd      #0xffff
    bne notEmpty
    leay     2,y
    dec      0,sp
    bne moreEmpty

    pula
    ldaa #0
    puly
rts

notEmpty:
    pula
    puly
    ldaa #1
rts
*/

/*
# B: Page
.align 2
checksumPage:

    # Load page
    ldab 10,sp

    # Set page
    ldx     #0x30
    stab     0,x

    # Pointer to checksum
    ldy #checksum

    # Clear checksum
    ldd      #0
    std      0,y
    std      2,y

    # Page window
    ldx #0x8000

sumMore:

    # Load flash byte
    ldaa     0,x

    # Add to lowest byte of checksum
    adda     3,y
    staa     3,y

    # add carry if present to the higher ones
    ldaa     2,y
    adca     #0
    staa     2,y

    ldaa     1,y
    adca     #0
    staa     1,y

    ldaa     0,y
    adca     #0
    staa     0,y

    leax     1,x
    tfr      x,d
    cmpd #0xC000
    bne sumMore

    # Indicate success
    ldd      #0
rts

.align 4
checksum: .long 0xFFFFFFFF
_padding: .long 0
*/
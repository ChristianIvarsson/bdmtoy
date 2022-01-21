.global startup
.global damnlinker
_start:
startup:

MD5Test:

    # Base address of parameters
    ldx     #0x2100

    # A
    ldd     #0x6745
    std     16,x
    ldd     #0x2301
    std     18,x

    # B
    ldd     #0xefcd
    std     20,x
    ldd     #0xab89
    std     22,x

    # C
    ldd     #0x98ba
    std     24,x
    ldd     #0xdcfe
    std     26,x

    # D
    ldd     #0x1032
    std     28,x
    ldd     #0x5476
    std     30,x


    # ppage @ 0xff
    # 8 pages
    ldab     #0

nextPage:
	pshb

	# Set page we want to access
	ldy     #0xFF
	stab     0,y

	# Size / 64
    ldd     #256
    pshd

    # Where to read from
    ldy     #0x8000

    jsr MD5Hash
    leas 2,sp


    ldd #0
    pulb
    addd #1

    cmpb #8
    bne nextPage


# Finalize hash
    leas   -64,sp
    leay     0,sp

    ldab    #64
    ldaa     #0

clearBuf:
	staa 0,y
    leay 1,y
    decb
    bne clearBuf

    leay     0,sp


    # Append one bit
    ldab    #0x80
    stab     0,y

    # 56
    # Store length, 0x20000, shifted 3 bits to the left...
    ldab    #0x10
    stab     58,y


    ldd #1
    pshd
    jsr MD5Hash
    leas 2,sp

    ldx #0

bgnd


# -:Extremely simple flash driver:-
# It performs sector erases automatically so there's no need to erase the flash beforehand
# It'll ONLY work on parts with 32 and 64K blocks. 128K has other requirements and will more than likely fail!

/*
# Commented out comments since my assembler is high or something..
# Data in:
# 10: Page / mask
# 08: Length
# 06: Flash Address (Within page window)
# 04: Buffer address
# 02: 1 -> temp -> 1
# sp:
*/
writeFlash:
# Set flash block and page
    # block = (0x3f-ppage)>>2
    # Load page
    ldab    10,sp
    ldx     #0x30
    stab     0,x

    # Block..
    ldaa    #0x3f
    sba
    lsra
    lsra
    anda    #7

    ldx     #0x100
    staa     3,x

    # negate locks
    ldaa    #0xFF
    staa     4,x

    # Load length and store a local copy
    ldd      8,sp
    std      2,sp

    # Load pointers
    ldx      4,sp /* buffer */
    ldy      6,sp /* Flash  */
    # bgnd

writeNext:
# Compare data, skip to next sector if data is identical
# Checks 256 words
    pshy
    pshx

    ldaa #0
    psha

verifmore:
    ldd      0,x
    cpd      0,y
    bne flashNotident
    leay     2,y
    leax     2,x
    dec      0,sp
    bne verifmore

    leas     5,sp
    lbra checkNext

flashNotident:
    leas     1,sp
    pulx
    puly

# Erase sector (512 bytes)
    pshx
    ldx     #0x100
eraseSect:

    # Store junk to indicate which sector
    std      0,y

    # Command: Sector erase
    ldaa    #0x40
    staa     6,x

    # Clear CBEIF
    ldaa    #0x80
    staa     5,x

    # Check PVIOL / ACCERR, clear if set (ie. retry and try again.)
    ldaa     5,x
    anda    #0x30
    beq checkCBEIF_e
    staa     5,x
    bra eraseSect

    # We only intend to send ONE command so no need
checkCBEIF_e:
    # ldaa     5,x
    # anda    #0x80
    # bne eraseSect

    # Wait for finish flag
checkCCIF_e:
    brclr 0x105,#0x40,#checkCCIF_e

    # Burst write 8 blocks
    ldab #8
    pshb

writeSector:

# Burst can write 64 bytes, we need 512
    ldab #32
    pshb
    # bgnd

burstWrite:
    ldx      2,sp  /* Buffer pointer */
    ldd      0,x   /* Load data      */
    std      0,y   /* Store data     */

    ldx     #0x100

    # Command: Write word
    ldaa    #0x20
    staa     6,x

    # Clear CBEIF
    ldaa     5,x
    oraa    #0x80
    staa     5,x

    # Check PVIOL / ACCERR, clear if set (Retry until we succeed)
    ldaa     5,x
    anda    #0x30
    beq contWrite
    staa     5,x
    bra burstWrite

contWrite:
    # Update pointers and counters..
    ldd      2,sp
    addd    #2
    std      2,sp
    leay     2,y
    dec      0,sp

    # Check if command queue is empty (Can take more commands)
checkCBEIF_w:
    ldaa     5,x
    anda    #0x80
    beq checkCCIF_w

# Some versions support burst flashing..
    ldaa     0,sp
    bne burstWrite

checkCCIF_w:
    # Wait for finish flag
    brclr 0x105,#0x40,#checkCCIF_w

    ldaa     0,sp
    bne burstWrite

    leas     1,sp
    dec      0,sp
    bne writeSector


/*
    pshy

    # Flash counts in words..
    # And does not care about pages the same way we do..
    # Take flash pointer, shift it up to our ways, decapitate higher bits (since it can count up to 32767), and add 0x8000 to reach page window
    tfr y,d
    lsld
    anda #0x3f
    oraa #0x80
    tfr d,y


    puly

    

writeSect:
    ldx      1,sp
    ldd      0,x
    ldx     #0x100
    std     10,x
    sty      8,x

    # Command: Write word
    ldaa    #0x20
    staa     6,x

    # Clear CBEIF
    ldaa     5,x
    oraa    #0x80
    staa     5,x

    # Check PVIOL / ACCERR, clear if set
    ldaa     5,x
    anda    #0x30
    beq contWrite
    staa     5,x
    bra writeSect

contWrite:
    # Update pointers and counters..
    ldd      1,sp
    addd    #2
    std      1,sp
    leay     1,y
    dec      0,sp

    # Check if command queue is empty (Can take more commands)
checkCBEIF_w:
    ldaa     5,x
    anda    #0x80
    beq checkCCIF_w

# Some versions support burst flashing..
    ldaa     0,sp
    bne writeSect

checkCCIF_w:
    ldaa     5,x
    anda    #0x40
    beq checkCCIF_w

    ldaa     0,sp
    bne writeSect
*/






    pulb
    pulx

checkNext:
    dec      2,sp
    dec      2,sp
    lbne writeNext
flashDone:

    ldd      #1
    std      2,sp
    sty      6,sp
bgnd

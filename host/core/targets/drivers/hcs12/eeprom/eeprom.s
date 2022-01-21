
# -:Extremely simple eeprom driver:-
# It performs sector erases automatically so there's no need to erase the eeprom beforehand
# It'll ONLY work on parts with 32 and 64K blocks. 128K has other requirements and will more than likely fail!

/*
# Commented out comments since my assembler is high or something..
# Data in:
# 08: Length
# 06: Eeprom Address (Within page window)
# 04: Buffer address
# 02: 1 -> temp -> 1
# sp:
*/
writeEeprom:

    # negate locks
    ldx     #0x110
    ldaa    #0xFF
    staa     4,x

    # Load length and store a local copy
    ldd      8,sp
    std      2,sp

    # Load pointers
    ldx      4,sp /* buffer */
    ldy      6,sp /* Eeprom */
    # bgnd

writeNext:
# Compare data, skip to next sector if data is identical
# Checks 2 words
    pshy
    pshx

    ldaa #2
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

# Erase sector (4 bytes)
    pshx
    ldx     #0x110
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

    # Wait for finish flag
    brclr 0x115,#0x40,#checkCBEIF_e

    # Write 2 words
    ldab #2
    pshb
#bgnd
writeSector:

    ldx      1,sp  /* Buffer pointer */
    ldd      0,x   /* Load data      */
    std      0,y   /* Store data     */

    ldx     #0x110

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
    bra writeSector

contWrite:
    # Update pointers and counters..
    ldd      1,sp
    addd    #2
    std      1,sp
    leay     2,y
    dec      0,sp

checkCCIF_w:
    # Wait for finish flag
    brclr 0x115,#0x40,#checkCCIF_w
    ldaa     0,sp
    bne writeSector

    pulb
    pulx

checkNext:
    ldd      2,sp
    addd    #0xFFFC
    std      2,sp
    lbne writeNext

    ldd      #1
    std      2,sp
    sty      6,sp
bgnd

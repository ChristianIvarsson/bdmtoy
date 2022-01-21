# r3: Do what (0 read, 1 write)
# r5: Dump to or write from this buffer
# r6: Continue for this number of bytes
# out: r3 1 = ok, !1 = Fail

Entry:
    # Preload global registers
    lis    r15,     0x0030
    li     r16,     0x5100
    or     r15,    r15, r16  /* Pointer to QSPI region */ 

    # Prepare queue
    addi    r7,    r15, 0xC0 /* Point to queue  */
    li      r9,     0xC8     /* 16-bit, hold cs */
    li      r8,     0x10
    mtctr   r8
KeepPrep:
    stbu    r9,   1(r7)
    bdnz KeepPrep

    li      r9,     0x68     /* 16-bit, release cs */
    stb     r9,   1(r7)

    # Set eeprom "address" to 0 and cheat in a read command in the upper half
    lis    r14,     0x03

    cmpwi   r3,     0x00
    bne WriteEEP
ReadEEP:

    bl ReadPage
    stswi  r24,     r5, 32 /* Push registers to buffer */
    addi    r5,     r5, 32 /* Increment buffer pointer */
    addi   r14,    r14, 32 /* Increment eeprom address */
    addic.  r6,     r6,-32 /* Decrement bytes left     */
    bne ReadEEP
    b OpSucc

WriteEEP:

    # Load buffer data to r16-r23
    lswi   r16,     r5, 32

WriteCompare:
    bl ReadPage

    # Err... I know
    cmpw   r16,    r24
    bne NotIdent
    cmpw   r17,    r25
    bne NotIdent
    cmpw   r18,    r26
    bne NotIdent
    cmpw   r19,    r27
    bne NotIdent
    cmpw   r20,    r28
    bne NotIdent
    cmpw   r21,    r29
    bne NotIdent
    cmpw   r22,    r30
    bne NotIdent
    cmpw   r23,    r31
    beq Wrident
NotIdent:
    bl WritePage
    b WriteCompare

Wrident:
    addi    r5,     r5, 32 /* Increment buffer pointer */
    addi   r14,    r14, 32 /* Increment eeprom address */
    addic.  r6,     r6,-32 /* Decrement bytes left     */
    bne WriteEEP

OpSucc:
    li      r3,   1
EntBDM:
    trap
    nop
    nop
    nop
    nop

OpFail:
    xor     r3,   r3, r3
    b EntBDM

# Read a full page (32 bytes), and store read data in r24-r31
ReadPage:
    li      r9,    -0x7738   /* ~0x88C8, 8-bit, hold cs |  16-bit, hold cs */
    sth     r9,0xC0(r15)     /* Store queue settings */
    lis     r9,     0x1100   /* Stop at pos 0x11     */
    stw    r14,0x80(r15)     /* [0] Command | [2] Address */
    li      r8,    -0x7FDD   /* ~0x8023              */
    stw     r9,-0xE4(r15)    /* Store length and reset queue pointer */
    sth     r8,-0xE6(r15)    /* Send it!             */

SPI_ReadWait:
    lhz     r9,-0xE2(r15)
    rlwinm. r9,     r9, 16, 7, 9
    beq SPI_ReadWait

    addi    r9,    r15, 0x44
    lswi   r24,     r9, 32
blr

WritePage:
    li      r9,     0x28     /* 8-bit, release cs    */
    li      r8,     0x06     /* "WREN"               */
    li     r10,    -0x7FDD   /* ~0x8023              */
    lis    r11,     0x00     /* A 0 is good to have  */
    sth     r9,0xC0(r15)     /* Store queue settings */
    sth     r8,0x80(r15)     /* [0] Command          */
    stw    r11,-0xE4(r15)    /* Store length and reset queue pointer */
    sth    r10,-0xE6(r15)    /* Send it!             */

SPS_WRENWait:
    lhz     r9,-0xE2(r15)
    rlwinm. r9,     r9, 16, 7, 9
    beq SPS_WRENWait

ActWrite:
    li      r8,     0x02     /* "WRITE"              */
    li      r9,    -0x7738   /* ~0x88C8, 8-bit, hold cs |  16-bit, hold cs */
    sth     r9,0xC0(r15)     /* Store queue settings */
    lis     r9,     0x1100   /* Stop at pos 0x11     */
    sth     r8,0x80(r15)     /* "WRITE"              */
    sth    r14,0x82(r15)     /* Address              */
    stw     r9,-0xE4(r15)    /* Store length and reset queue pointer */
    addi    r9,    r15, 0x84 /* Push data to tranram */
    stswi  r16,     r9, 32
    sth    r10,-0xE6(r15)    /* Send it!             */

SPS_WriteWait:
    lhz     r9,-0xE2(r15)
    rlwinm. r9,     r9, 16, 7, 9
    beq SPS_WriteWait

    # Wait for "Write In Progress" to negate
ReadStatus:
    li      r9,     0x68     /* 16-bit, release cs   */
    stb     r9,0xC0(r15)     /* Store queue settings */
    li      r9,     0x0500   /* "Read status"        */
    sth     r9,0x80(r15)     /* [0] Command          */
StatusShortcut:
    stw    r11,-0xE4(r15)    /* Store length and reset queue pointer */
    sth    r10,-0xE6(r15)    /* Send it!             */

SPS_StatusWait:
    lhz     r9,-0xE2(r15)
    rlwinm. r9,     r9, 16, 7, 9
    beq SPS_StatusWait
    lbz     r9,0x41(r15)
    andi.   r9,     r9, 1
    bne StatusShortcut
blr

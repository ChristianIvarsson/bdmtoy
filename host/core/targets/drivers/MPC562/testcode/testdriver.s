# r3: Do what (0 init, 2 erase, 1 write, ..)
# r4: Base of device to perform operation on
# r5: Start operation from this address
# r6: Continue for this number of bytes
# r7: Buffer pointer
# out: r3 1 = ok, !1 = Fail, r4 = reason if failed, otherwise still base address

Entry:
    cmpwi   r3,   1
    bne OtherCommands
WriteFlash:
    subi    r5,   r5, 4
    add     r8,   r5, r6 /* "Calc" last address  */
    mr     r22,   r7     /* Copy buffer pointer  */

VerifWrite:
    # Return to read
    stw    r20, 4(r5) 
    subi   r22,  r22, 4

VerifLoopWrite:
    cmpw    r8, r5
    ble WriteDone
    lwzu    r4, 4( r5) /* Flash  */
    lwzu    r9, 4(r22) /* Buffer */
    cmpw    r9, r4
    beq VerifLoopWrite

WriteDword:
    stw    r21, 0(r5) /* Command */
    stw     r9, 0(r5) /* Data    */
ReadStsWr:
    lwz     r9, 0(r5) /* Status  */

    # Flash bit 7 (Processor bit 9)
    rlwinm. r10, r9, 0, 8, 10
    beq ReadStsWr

    # Flash bit 1 (Processor bit  5) /* Protected error */
    # Flash bit 3 (Processor bit  0) /* VPP missing error */
    andis. r10,r9, 0x8400
    bne OpFail

    # Flash bit 4 (Processor bit 26) /* Gen. write error, retry */
    andi.  r10,r9, 0x20
    bne ClrStW

    subi    r5,   r5, 4
    b VerifWrite

ClrStW:
    li      r9,   0xA0 /* Clear status   */
    stw     r9, 0(r5)
    stw    r20, 0(r5)  /* Return to read */
    b WriteDword

WriteDone:
    addi    r5,   r5, 4
OpSucc:
    li      r3,   1
EntBDM:
    trap
    nop
    nop
    nop
    nop

OtherCommands:    
    cmpwi   r3,   0
    beq InitFlash
    cmpwi   r3,   2
    beq EraseFlash
OpFail:
    xor     r3,   r3, r3
    b EntBDM

InitFlash:

    isync
    isync
    isync
    
    # Negate Write Protect, Just in case
    lis     r9,   0x30
    addi    r9,   r9, -0x3F00
    lwz     r8, 0(r9)
    rlwinm  r8,   r8, 0, 24, 22 /* Clear bit 23, WP */
    stw     r8, 0(r9)

    # Clear status register
    li      r8,   0xA0
    stw     r8, 0(r4)

    # Preload common shortcuts
    lis    r21,   0xC540  /* Shortcut for return to read (FF) */
    li     r20,   0xA8
    or     r20,  r20, r21
    li     r21,   0x80   /* Send write command */

    b OpSucc

EraseFlash:
    mr     r11,   r5     /* Backup first address */
    add     r8,   r5, r6 /* "Calc" last address  */
    subi    r8,   r8, 4  /* ... */
    lis    r23,   0x0040 /* Shortcut for "confirm" (D0) */
    li     r22,   0x00A0
    or     r22,  r22, r23 
    li     r23,   0      /* Shortcut for 0xFFFF FFFF */
    subi   r23,  r23, 1

VerifE:
    # Return to read
    stw    r20, 0(r11)
    subi   r11,   r11, 4

VerifLoopE:
    cmpw    r8, r11
    ble OpSucc
    lwzu    r9, 4(r11)
    cmpw    r9, r23
    beq VerifLoopE

SndEr:
    # Clear status register
    li      r9,   0xA0
    stw     r9, 0(r11)

    # Return to read
    stw    r20, 0(r11)

    # Send erase command
    li      r9,   8
    stw     r9, 0(r11) /* Command */
    stw    r22, 0(r11) /* Confirm */

RdStatE:
    lwz     r9, 0(r11)

    # Flash bit 7 (Processor bit 9)
    rlwinm. r10, r9, 0, 8, 10
    beq RdStatE
    
    # Flash bit 1 (Processor bit 5) /* Protected error */
    # Flash bit 3 (Processor bit 0) /* VPP missing error */
    andis. r10,r9, 0x8400
    bne OpFail            

    # Flash bit 4-5 (Processor bit 26,28) /* Command seq error */
    andi.  r10,r9, 0x28
    cmpwi  r10, 0x28
    beq OpFail

    # Flash bit 5 (Processor bit 28) /* Generic erase error, try again */
    andi.  r10,r9, 0x8
    bne SndEr

    b VerifE

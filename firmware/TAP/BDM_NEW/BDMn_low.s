.thumb
.global BDMNEW_DMADumpAS_USB_type2
.global BDMNEW_DMAFillAS
.align 8

# # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # #
# r4   : Returned status bits
# r5,r7: Temp
.macro ShiftHeaderDump

# Wait for target ready
    # mov    r5, #0x40
1:
    ldrb   r7, [r3, #5]
    lsl    r7, #25
    bmi 1b

    # Slower
    # ubfx   r7,  r7, #6, #1
    # cmp    r7, #0
    # bne 1b

    # Slow
    # and    r7, r5
    # bne 1b

    nop

# 4

# Start bit
    mov    r4, #0x80      /* Load TDI pin       */
    strb   r4, [r3, #12]  /* Set TDI high       */
    mov    r7, #0x20      /* Load clock pin     */
    strb   r7, [r3, #12]  /* Clock high         */
    mov    r4, #0xA0      /* Load clock and TDI */ /* Port operations are slow so this is still faster despite loading more data */
    strb   r4, [r3, #16]  /* Both low           */

    nop
    nop
    nop
    # nop

# Bit 0
    ldrb   r4, [r3, #5 ]  /* Read TDO   */
    strb   r7, [r3, #12]  /* Clock high */
    strb   r7, [r3, #16]  /* Clock low  */
    lsr    r4, #6

# Bit 1
    ldrb   r5, [r3, #5 ]  /* Read TDO   */
    strb   r7, [r3, #12]  /* Clock high */
    strb   r7, [r3, #16]  /* Clock low  */
    lsr    r5, #5
    orr    r4, r5

    # Currently not necessary due to pin-usage (bit #7 is tdi, which is guaranteed to be 0)
    # mov    r7, #3
    # and    r4, r7

.endm

# r4   : Returned status bits
# r5,r7: Temp
.macro ShiftHeaderDownload

1:
    ldrb   r7, [r3, #5]
    lsl    r7, #25
    bmi 1b

    # Voodoo
    # nop

# 4

# Start bit
    mov    r5, #0x80      /* Load TDI pin       */
    strb   r5, [r3, #12]  /* Set TDI high       */
    mov    r7, #0x20      /* Load clock pin     */
    strb   r7, [r3, #12]  /* Clock high         */
    mov    r4, #0xA0      /* Load clock and TDI */ /* Port operations are slow so this is still faster despite loading more data */
    strb   r4, [r3, #16]  /* Both low           */

    # Need a tiny delay
    nop
    nop
    nop

# Bit 0
    ldrb   r4, [r3, #5 ]  /* Read TDO   */
    strb   r7, [r3, #12]  /* Clock high */
    strb   r7, [r3, #16]  /* Clock low  */
    strb   r5, [r3, #12]  /* TDI high   */
    lsr    r4, #1

# Bit 1
    ldrb   r5, [r3, #5 ]  /* Read TDO   */
    strb   r7, [r3, #12]  /* Clock high */
    strb   r7, [r3, #16]  /* Clock low  */
    # lsr    r5, #5
    orr    r4, r5

    # Mask off TDI from read data
    bfc    r4, #7 , #1

.endm

.macro SPI_DMAPerform

    # Configure GPIO as SPI
    ldr    r7, [sp]
    str    r7, [r3]

    # Enable Channel 5 (Tx)
    ldr    r7, [sp, #20]
    str    r7, [r2, #20]
1:
    # Read counter for channel 4 (Rx)
    ldr    r7, [r2, #4]
    cmp    r7, #0
    bne 1b

    # Voodoo
    nop

    # Disable Channel 4 (Rx)
    ldr    r7, [sp, #8 ]
    str    r7, [r2, #0 ]

     # Disable Channel 5 (Tx)
    ldr    r7, [sp, #16]
    str    r7, [r2, #20]

    # Configure GPIO as GPIO
    str    r6, [r3]
.endm

BDMNEW_DMADumpAS_USB_type2:
    push  {r4 - r7, lr}
    sub    sp, #44

    # buffer[0] pointer
    # Ugly thing to make 'stm' work in a predictable way
    # There's room to spare both ways so no need to worry about over/'under'-flow
    lsr    r0, #2
    lsl    r0, #2
    add    r0, #2
    str    r0, [sp, #32]

    # Current address
    str    r1, [sp, #24]

    # Total length (In Dwords)
    lsr    r2, #2
    str    r2, [sp, #28]

    ldr    r1, = wizarr_
    ldr    r2, [r1, #12]
    ldr    r3, [r1, #16]

    ldr    r6, [r3]     /* Current gpio settings */
    ldr    r5, [r1, #8] /* gpio as spi           */
    orr    r5,  r6
    str    r5, [sp]

    mov    r5, #1

    ldr    r7, [r2, #0 ] /* DMA4    */
    bfc    r7, #0 , #1
    str    r7, [sp, #8 ] /* Stock   */
    orr    r7,  r5
    str    r7, [sp, #12] /* Enabled */

    ldr    r7, [r2, #20] /* DMA5    */
    bfc    r7, #0 , #1
    str    r7, [sp, #16] /* Stock   */
    orr    r7,  r5
    str    r7, [sp, #20] /* Enabled */

    # 0x100 dwords, 1024 bytes
    lsl    r5,  #8
    str    r5, [sp, #40]

.align 4
type2NewFrame:

    ldrh   r5, [sp, #40] /* Load default number of dwords to dump in one frame */
    ldr    r7, [sp, #28] /* Length left, total */

    # Check if default length is too much
    cmp    r5,  r7
    ble noDwordsFine
    mov    r5,  r7

noDwordsFine:

    # Store intended length
    strh   r5, [sp, #36]

    # Total length--
    sub    r7,  r5
    str    r7, [sp, #28]

# Store header

    # Length of frame[0]
    lsl    r7, r5, #1
    add    r7, #5     /* Include header itself */
    strh   r7, [r0]

    # Command[1] / Status[2]
    add    r0, #2
    mov    r4, #0x50     /* TAP_DO_DUMPMEM  */
    ldr    r7, [sp, #24] /* Current address */
    stm    r0!,{r4}      /* Store command and status */
    str    r7, [r0]      /* Store address   */

    # Address++
    lsl    r5, #2
    add    r7,  r5
    str    r7, [sp, #24]

# Grew tired of counting instructions. This should force insertion of nop's if it's unaligned
.align 4
type2MoreWords:

# Send instruction 1

    mov    r7, #2
    add    r0,  r0, #4   /* Inc data  */
    str    r0, [r2, #12] /* CMAR , Rx */
    str    r7, [r2, #4 ] /* CNDTR, Rx */
    str    r1, [r2, #32] /* CMAR , Tx */
    str    r7, [r2, #24] /* CNDTR, Tx */

    # Prime DMA 1, Channel 4 (Rx)
    ldr    r4, [sp, #12]
    str    r4, [r2, #0 ]

    # Voodoo
    nop

    ShiftHeaderDump
    SPI_DMAPerform

    # Should be 3: "NULL"
    cmp    r4, #3
    bne type2Fail

# Send instruction 2

    mov    r7,  #2
    add    r5,  r1, #4
    str    r0, [r2, #12] /* CMAR , Rx */
    str    r7, [r2, #4 ] /* CNDTR, Rx */
    str    r5, [r2, #32] /* CMAR , Tx */
    str    r7, [r2, #24] /* CNDTR, Tx */

    # Prime DMA 1, Channel 4 (Rx)
    ldr    r4, [sp, #12]
    str    r4, [r2, #0 ]

    ShiftHeaderDump
    SPI_DMAPerform

    # Status should be 0 here
    cbnz r4, type2Fail

    # Voodoo..
    nop
    nop
    # nop

    ldrh   r7, [sp, #36]
    sub    r7, #1
    strh   r7, [sp, #36]
    bne type2MoreWords

    ldr    r0, [sp, #32]  /* Load start of buffer */

    push  {r0-r3}
    blx usb_sendData
    cmp    r0, #0
    pop   {r0-r3}
    bne type2Fail

    ldrh   r7, [sp, #28]
    cbz    r7, type2Done
    b type2NewFrame

type2Done:
    eor    r0,  r0
    add    sp, #44
    pop   {r4 - r7}
    pop   {r1}
bx r1

type2Fail:
    mov    r0, #0xFF
    add    sp, #44
    pop   {r4 - r7}
    pop   {r1}
bx r1


# 3249 mS
# 1250
BDMNEW_DMAFillAS:
    push  {r4 - r7, lr}
    sub    sp, #28

    # Total length (In Dwords)
    lsr    r1, #2

    ldr    r7, = wizarr_
    ldr    r2, [r7, #12]
    ldr    r3, [r7, #16]

    ldr    r6, [r3]     /* Current gpio settings */
    ldr    r5, [r7, #8] /* gpio as spi           */
    orr    r5,  r6
    str    r5, [sp]

    ldr    r7, [r7, #24]
    str    r7, [sp, #24]

    mov    r5, #1

    ldr    r7, [r2, #0 ] /* DMA4    */
    bfc    r7, #0 , #1
    str    r7, [sp, #8 ] /* Stock   */
    orr    r7,  r5
    str    r7, [sp, #12] /* Enabled */

    ldr    r7, [r2, #20] /* DMA5    */
    bfc    r7, #0 , #1
    str    r7, [sp, #16] /* Stock   */
    orr    r7,  r5
    str    r7, [sp, #20] /* Enabled */

# Grew tired of counting instructions. This should force insertion of nop's if it's unaligned
.align 4
DMAFillMore:

# Send it!

    mov    r7, #2
    str    r0, [r2, #32] /* CMAR , Tx */
    add    r0,  r0, #4   /* Inc data  */
    str    r7, [r2, #24] /* CNDTR, Tx */

    ShiftHeaderDownload

    # Configure GPIO as SPI
    ldr    r7, [sp]
    str    r7, [r3]

    # Enable Channel 5 (Tx)
    ldr    r7, [sp, #20]
    str    r7, [r2, #20]

StillSending:
    # Read counter for channel 5 (Tx)
    ldr    r7, [r2, #24]
    cmp    r7, #0
    bne StillSending

    # Ugly as frig..
    mov    r5, #0x80
StillBusy:
    ldr    r7, [sp, #24]
    ldrh   r7, [r7]
    and    r7, r5
    bne StillBusy

     # Disable Channel 5 (Tx)
    ldr    r7, [sp, #16]
    str    r7, [r2, #20]

    # Configure GPIO as GPIO
    str    r6, [r3]

    # Should be 3: "NULL"
    cmp    r4, #0x59
    ble FillFail

    sub    r1, #1
    bne DMAFillMore

    mov    r5, #0x80      /* Load TDI pin       */
    strb   r5, [r3, #16]  /* Set TDI low        */
    eor    r0,  r0
    add    sp, #28
    pop   {r4 - r7}
    pop   {r1}
bx r1

FillFail:
    mov    r5, #0x80      /* Load TDI pin       */
    strb   r5, [r3, #16]  /* Set TDI low        */
    mov    r0, #0xFF
    add    sp, #28
    pop   {r4 - r7}
    pop   {r1}
bx r1



#               0           4           8               12          16          20          24
wizarr_:  .long 0x9BA67C16, 0x00048401, 0x80800000,     0x40020044, 0x40010C04, 0x40006014, 0x40003808

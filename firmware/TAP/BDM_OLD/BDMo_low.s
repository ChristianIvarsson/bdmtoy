# If you see read_TDO in weird places it's there to pad timings
.global BDMold_shift
.global BDMold_turbodump
.global BDMold_turbofill
.thumb

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Load value, jump here and return: 12 / 6 (r4: 1) Branch prediction will take 6 cycles from this with larger delays..
# 6 DWT cycles+ for every loop
r1Delay:
    add r2, #-1
    bne r1Delay
bx lr

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
.macro set_BOTH_low
    ldr    r2, [r0, #12] /* load ptr                    */
    mov    r3, #0xA0     /* It's actually 0xA000...     */
    strb   r3, [r2]      /* Set pin low                 */
.endm

.macro set_CLK_high
    ldr    r2, [r0, #8]  /* load ptr                    */
    mov    r3, #0x20     /* It's actually 0x2000...     */
    strb   r3, [r2]      /* Set pin high                */
.endm

.macro set_CLK_low
    ldr    r2, [r0, #12] /* load ptr                    */
    mov    r3, #0x20     /* It's actually 0x2000...     */
    strb   r3, [r2]      /* Set pin low                 */
.endm

.macro set_TDI_high
    ldr    r2, [r0, #8]  /* load ptr                    */
    mov    r3, #0x80     /* It's actually 0x8000...     */
    strb   r3, [r2]      /* Set pin high                */
.endm

.macro set_TDI_low
    ldr    r2, [r0, #12] /* load ptr                    */
    mov    r3, #0x80     /* It's actually 0x8000...     */
    strb   r3, [r2]      /* Set pin low                 */
.endm

.macro read_TDO
    ldr    r2, [r0, #16] /* load ptr                    */
#   mov    r3,  #1       /* Preload value to be AND'ed  */
    ldrb   r2, [r2,#1]   /* Load pin state              */
    lsr    r2,  #6       /* Shift down pin 14 to bit 0  */
#   and    r2,  r3       /* AND result with 1           */
    # Normally you'd need to AND the result but since data is only read when shifting out 0's, we know data out will be 0 (data out is pin 15)
    # Needless to say this will bite your ass if the pinout is changed
.endm

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# DWT timer: Not used now that the code is profiled.
# Extremely useful for tweaking delays since it's run at the same speed as the processor.
.macro dwt_Capture
    ldr    r3, [r0, #20] /* Fetch pointer to DWT clock  */
    ldr    r3, [r3]      /* Capture current tick        */
    str    r3, [r0, #24] /* Store result     */
    nop                  /* Inserting nops makes sure we have a known good base value */
    nop                  /* (One cycle would magically come and go with other instructions) */
    nop
.endm

.macro dwt_Recapture
    nop
    nop
    nop
    ldr    r3, [r0, #20] /* Fetch pointer to DWT clock */
    ldr    r2, [r3]      /* Capture new time */
    ldr    r3, [r0, #24] /* Fetch old result */
    sub    r2,  r2, r3   /* Calculate delta  */
    sub    r2, #11       /* Subtract the time it took to capture these values */
    str    r2, [r0, #24] /* Store result     */
.endm

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# This thing can go from 0 to 1 MHz ~ish (Due to configurable speeds we can't go particularly fast..)
#00: payload
#04: delayKeepState
#08: *pinSetPtr
#12: *pinClrPtr
#16: *pinRdPtr
#20: *dwtptr
#24: benchTime
BDMold_shift:

    push  {r1 - r7, lr}  /* Usual stack stuff           */

    mov    r7, #16       /* Number of bits to send      */
    ldr    r6, [r0, #0 ] /* Load data                   */
    lsl    r6, #15       /* Left-align package         */

    # dwt_Capture

/* * * * * * * * * * * * * * * * * * * * * * */

    # Take care of attention bit
    # Shift out 36(30)
    set_BOTH_low
    nop
    ldr    r2, [r0, #4]
    bl r1Delay
    read_TDO
    read_TDO
    mov    r5,  r2

    # Latch data out 36(30)
    set_CLK_high
    nop
    ldr    r2, [r0, #4]
    bl r1Delay
    read_TDO
    read_TDO
    nop

    # Shift out/in data 36(30)
shiftMore:
    set_CLK_low

    lsl    r6, #1
    bpl  setZero

    set_TDI_high
    b   oneSet

setZero:
    set_TDI_low
    nop
    nop

oneSet:

    ldr    r2, [r0, #4]
    bl r1Delay

    read_TDO
    orr    r6,  r2

    # Latch data out 36(30)
    set_CLK_high

    read_TDO
    ldr    r2, [r0, #4]
    bl r1Delay
    read_TDO

    sub    r7,  #1       /* Subtract number of bits left */
    bne shiftMore

/* * * * * * * * * * * * * * * * * * * * * * */

    # dwt_Recapture

    str    r6, [r0, #0 ] /* Save data          */
    mov    r0,  r5

    # Stack stuff..
    pop   {r1 - r7}
    pop   {r1}

bx r1

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# This thing has delays timed for 4 MHz BDM (Min target speed ~8 MHz) but SPI can go significantly faster depending on what you set it to.
# Protocol is 1 + 16 bits (status and actual data). Our delays takes care of the status bit while SPI does the heavy lifting.
// 00: *spiSrPntr
// 04: *spiDataPntr
// 08: *pinCrhPtr

// 12: *pinClrPtr
// 16: *pinRdPtr
// 20: *dwt_pntr
// 24: dwt_time
// 28: dataPtr;
// 32: noDwords
BDMold_turbodump:

    push  {r4 - r7, lr}  /* Usual stack stuff           */

    ldr    r4, [r0, #8 ]
    ldr    r5, [r4, #0 ] /* Stock */
    ldr    r7, [r0, #28] /* Pointer */

/* * * * * * * * * * * * * * * * * * * * * * */

AttentionHigh:

    # We could store this but we need a delay of 30 cycles anyway..
    set_BOTH_low
    mov    r6, #0x80
    lsl    r6, #8
    mov    r2, #0x80
    orr    r6,  r2
    lsl    r6, #16
    orr    r6,  r5

    nop
    nop

    read_TDO
    str    r6, [r4, #0]
    mov    r1,  r2

    ldr    r2, [r0, #4]
    mov    r3,  #0
    strh   r3, [r2, #0]

    mov    r3, #1
BusyHigh:
    ldr    r6, [r0, #0]
    ldrh   r6, [r6, #0]
    ror    r6, r3
    bpl BusyHigh

    # Fetch high word
    ldrh   r6, [r2, #0]
    str    r5, [r4, #0]

    ror    r1, r3
    bmi AttentionHigh

    # Store data
    strh   r6, [r7, #2]

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

    set_BOTH_low

    mov    r6, #0x80
    lsl    r6, #8
    mov    r2, #0x80
    orr    r6,  r2
    lsl    r6, #16
    orr    r6,  r5

    # Dump32: 0x1D80
    mov    r3, #0x1D
    lsl    r3, #8
    orr    r3,  r2

    # Enable SPI
    str    r6, [r4, #0]

    # Store next command..
    ldr    r2, [r0, #4]
    strh   r3, [r2, #0]

    mov    r3, #1
BusyLow:
    ldr    r6, [r0, #0]
    ldrh   r6, [r6, #0]
    ror    r6, r3
    bpl BusyLow

    ldrh   r6, [r2, #0]
    str    r5, [r4, #0]

# http://www.bitsavers.org/components/motorola/68000/CPU32_Reference_Manual_Aug90.pdf
    # There's no need to read the second status #

/* * * * * * * * * * * * * * * * * * * * * * */

    # Store data
    strh   r6, [r7, #0]
    # Increment pointer
    add    r7, #4

    # We need a slight delay for it to register the command..
    # 5: Stable down to 14.680
    mov    r2, #5
    bl r1Delay

    ldr    r2, [r0, #32]
    sub    r2, #1
    str    r2, [r0, #32]
    bne AttentionHigh

    # Store pointer..
    # str    r7, [r0, #28]

    # Stack stuff..
    pop   {r4 - r7}
    pop   {r1}

bx r1









# Sequence:
# 1. Send command
# 2. Send data high
# 3. Send data low

# 4. send

BDMold_turbofill:

    push  {r4 - r7, lr}  /* Usual stack stuff           */

    ldr    r4, [r0, #8 ]
    ldr    r5, [r4, #0 ] /* Stock */
    ldr    r7, [r0, #28] /* Pointer */

    b inFill
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Command
/*
    set_BOTH_low
    mov    r6, #0x80
    lsl    r6, #8
    mov    r2, #0x80
    orr    r6,  r2
    lsl    r6, #16
    orr    r6,  r5

    # Fill32
    # 0x1C80
    mov    r1, #0x1C
    lsl    r1, #8
    orr    r1,  r2

    # nop
    # nop

    # Enable SPI
    str    r6, [r4, #0]

    # Fetch pointer to data register and store command
    ldr    r2, [r0, #4]
    strh   r1, [r2, #0]

    mov    r3, #1
BusyCommandFill:
    ldr    r6, [r0, #0]
    ldrh   r6, [r6, #0]
    ror    r6, r3
    bpl BusyCommandFill

    ldrh   r2, [r2, #0]
    str    r5, [r4, #0]
*/
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# High word
FillMore:

    # Slight delay. Let it register what just happened
    # mov    r2, #1
    # bl r1Delay

    # We could store this but we need a delay of 30 cycles anyway..
    set_BOTH_low
    mov    r6, #0x80
    lsl    r6, #8
    mov    r2, #0x80
    orr    r6,  r2
    lsl    r6, #16
    orr    r6,  r5

    # nop
    # nop

    str    r6, [r4, #0]

    ldr    r2, [r0, #4]
    ldrh   r3, [r7, #2]
    strh   r3, [r2, #0]

    mov    r3, #1
    ldr    r1, [r0, #0]
BusyHighFill:
    ldrh   r6, [r1, #0]
    ror    r6, r3
    bpl BusyHighFill

    # Fetch only to make SPI happy
    ldrh   r6, [r2, #0]
    str    r5, [r4, #0]

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Low word

    set_BOTH_low

    mov    r6, #0x80
    lsl    r6, #8
    mov    r2, #0x80
    orr    r6,  r2
    lsl    r6, #16
    orr    r6,  r5

    # Enable SPI
    str    r6, [r4, #0]

    # Store data..
    ldr    r2, [r0, #4]
    ldrh   r3, [r7, #0]
    strh   r3, [r2, #0]

    mov    r3, #1
    ldr    r1, [r0, #0]
BusyLowFill:
    ldrh   r6, [r1, #0]
    ror    r6, r3
    bpl BusyLowFill

    # Fetch only to make SPI happy
    ldrh   r6, [r2, #0]
    str    r5, [r4, #0]

    # 2
    # mov    r2, #2
    # bl r1Delay

    # Increment data pointer
    add    r7, #4

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

    # Fetch number of words left, If we have more, embedd fill to speed up the process
    ldr    r2, [r0, #32]
    sub    r2, #1
    str    r2, [r0, #32]
    beq lastFill

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# In busyfill
inFill:

    # We could store this but we need a delay of 30 cycles anyway..
    set_BOTH_low
    mov    r6, #0x80
    lsl    r6, #8
    mov    r2, #0x80
    orr    r6,  r2
    lsl    r6, #16
    orr    r6,  r5

    nop
    nop

    read_TDO
    str    r6, [r4, #0]
    mov    r1,  r2

    # Fill32
    # 0x1C80
    mov    r3, #0x1C
    lsl    r3, #8
    mov    r2, #0x80
    orr    r3,  r2
    ldr    r2, [r0, #4]
    strh   r3, [r2, #0]

    mov    r3, #1
BusyInFill:
    ldr    r6, [r0, #0]
    ldrh   r6, [r6, #0]
    ror    r6, r3
    bpl BusyInFill

    # Fetch high word
    ldrh   r6, [r2, #0]
    str    r5, [r4, #0]

    ror    r1, r3
    bmi inFill

    b FillMore
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# last busyfill
lastFill:

    # We could store this but we need a delay of 30 cycles anyway..
    set_BOTH_low
    mov    r6, #0x80
    lsl    r6, #8
    mov    r2, #0x80
    orr    r6,  r2
    lsl    r6, #16
    orr    r6,  r5

    nop
    nop

    read_TDO
    str    r6, [r4, #0]
    mov    r1,  r2

    mov    r3, #0
    ldr    r2, [r0, #4]
    strh   r3, [r2, #0]

    mov    r3, #1
BusyLastFill:
    ldr    r6, [r0, #0]
    ldrh   r6, [r6, #0]
    ror    r6, r3
    bpl BusyLastFill

    # Fetch high word
    ldrh   r6, [r2, #0]
    str    r5, [r4, #0]

    ror    r1, r3
    bmi lastFill


    # Store data
    # strh   r6, [r7, #0]
    # Increment pointer


    # We need a slight delay for it to register the command..
    # 5: Stable down to 14.680




    # Store pointer..
    # str    r7, [r0, #28]

    # Stack stuff..
    pop   {r4 - r7}
    pop   {r1}

bx r1









/*

*/

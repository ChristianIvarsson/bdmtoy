# # # # # # # # # # #
# Global declarations

.global sBDMsendBits
.global sBDMreceiveBits
.global sBDMextraDelay

.thumb

# # # # # # # # # # # # # # # #
# One bit time is 16 target ticks
# Send:
# Configure drive as push-pull
# Tick  0: Bring pin low
# Tick  4: Set pin according to data
# Tick 13: Raise
# Tick 16: New period
# Restore drive to open-drain

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

/*
 *  00 payload       // Data in and out. Remember to shift up data to be sent since we're MSB first!
 * +04 noBits        // Number of bits to send or receive
 * +08 delayKeepLow  // Add this many loops to keep bkgnd low for FOUR target ticks.
 * +12 delayToHigh   // Add this many loops to bring bkgnd high after NINE target ticks.
 * +16 delayKeepHigh // Add this many loops to keep bkgnd high for THREE target ticks.
 * +20 delaySample   // Add this many loops so that we sample bkgnd SIX target ticks after we released the pin.
 * +24 pinCfgMASK;   // Mask to use when configuring pin register
 * +28 *pinCfgPtr;   // Register to configure pin behaviour
 * +32 *pinClrPtr;   // Register to drag pin low
 * +36 *pinSetPtr;   // Register to set/allow pin high
 * +40 *pinPtr       // Register to read pin status
 * +44 *dwtptr       // Pointer to dwt timer
 * +48 benchTime     // Store resulting time
*/

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# GPIOB->CRH = (GPIOB->CRH & (~(0xF << (4 * 4)))) | (0b0001 << (4 * 4)); // Set as push-pull
# GPIOB->CRH = (GPIOB->CRH & (~(0xF << (4 * 4)))) | (0b0110 << (4 * 4)); // Set as open-drain
setPushPull:
    ldr    r2, [r0, #24] /* load mask                   */
    ldr    r3, [r0, #28] /* load ptr                    */

    mov    r4,  #0x1     /* Load data to be OR'ed       */
    lsl    r4,  #20      /* Move it to correct location */

    ldr    r5, [r3]      /* Load current cfg            */
    and    r5,  r2       /* AND current with mask       */
    orr    r5,  r4       /* OR in config for pin        */
    str    r5, [r3]      /* Store new cfg               */
bx lr

setOpenDrain:
    ldr    r2, [r0, #24] /* load mask                   */
    ldr    r3, [r0, #28] /* load ptr                    */

    mov    r4,  #0x6     /* Load data to be OR'ed       */
    lsl    r4,  #20      /* Move it to correct location */

    ldr    r5, [r3]      /* Load current cfg            */
    and    r5,  r2       /* AND current with mask       */
    orr    r5,  r4       /* OR in config for pin        */
    str    r5, [r3]      /* Store new cfg               */
bx lr


# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
.macro setPinHigh
    ldr    r2, [r0, #36] /* load ptr                    */
    mov    r3, #0x20     /* It's actually 0x1000...     */
    lsl    r3, #0x08     /* Since we're in gimped mode  */
    str    r3, [r2]      /* Set pin high                */
.endm

.macro setPinLow
    ldr    r2, [r0, #32] /* load ptr                    */
    mov    r3, #0x20     /* It's actually 0x1000...     */
    lsl    r3, #0x08     /* Since we're in gimped mode  */
    str    r3, [r2]      /* Set pin low                 */
.endm

.macro readPin
    ldr    r2, [r0, #40] /* load ptr                    */
    mov    r3,  #1       /* Preload value to be AND'ed  */
    ldr    r2, [r2]      /* Load pin state              */
    lsr    r2,  #13      /* Shift down pin 13 to bit 0  */
    and    r2,  r3       /* AND result with 1           */
.endm

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

# inherent delay 8 - 14
# 0: 9
# 1: 8
# 2: 14
# 3: 13
# 4: 12
# 5: 11
# 6: 10
# 7: 9
# 8: 8
# 9: 14
# 100: 14

# r2: no ticks to wait
.macro dwt_Wait
    ldr    r3, [r0, #44] /* Fetch pointer to DWT clock  */
    ldr    r4, [r3]      /* Capture current tick        */
10:
    ldr    r5, [r3]      /* Capture current tick        */
    sub    r5,  r5, r4
    cmp    r5, r2
    bmi 10b
.endm

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
sBDMsendBits:

    # Disable interrupts. Do this early due to the pipeline
    cpsid i

    push  {r1 - r7, lr}  /* Usual stack stuff           */

    ldr    r7, [r0, #4 ] /* Number of bits to send      */
    ldr    r6, [r0, #0 ] /* Load data                   */

    mov    r5, #32       /* Left-align data             */
    sub    r5,  r7
    lsl    r6,  r5

    bl setPushPull

    # r7: No bits
    # r6: Data
    # 2-5: thrash
    # r1:
    # r0: data ptr

    # nop                  /* To get accurate timings..   */
    # nop

    # ldr    r3, [r0, #44] /* Fetch pointer to DWT clock  */
    # ldr    r1, [r3]      /* Capture current tick        */
    # nop                  /* Inserting nops makes sure we have a known good base value */
    # nop                  /* (One cycle would magically come and go with other instructions) */

bitCycleSend:

    # # # Drive clock low
    # 16 target clocks ~
    setPinLow

    ldr r2, [r0, #8]
    dwt_Wait


    # Set data
    # 19
    mov  r6, r6
    bmi  setOne

    setPinLow
    b   wasZero

    # This is wasting storage but we need it to keep timings the same
setOne:

    setPinHigh

wasZero:

    # Hold pin in this state for 9 target ticks
    ldr r2, [r0, #12]
    dwt_Wait

    # Set to high
    # 16
    setPinHigh
    ldr r2, [r0, #16]
    dwt_Wait


    # Subtract number of bits left
    lsl    r6,  #1
    sub    r7,  #1
    bne bitCycleSend
    # Enable interrupts
    cpsie i
/* * * * * * * * * * * * * * * * * * * * * * */
    # ldr    r3, [r0, #44] /* Fetch pointer to DWT clock */
    # ldr    r2, [r3]      /* Capture new time */
    # sub    r2,  r2, r1   /* Calculate delta  */
    # sub    r2, #6        /* Subtract the time it took to capture these values */
    # str    r2, [r0, #48] /* Store result     */

    bl setOpenDrain

    # Stack stuff..
    pop   {r1 - r7}
    pop   {r0}

bx r0


# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
sBDMreceiveBits:

    # Disable interrupts. Do this early due to the pipeline
    cpsid i

    push  {r1 - r7, lr}  /* Usual stack stuff           */

    ldr    r7, [r0, #4 ] /* Number of bits to receive   */
    mov    r6, #0        /* Load data                   */

    bl setOpenDrain
    # nop                  /* To get accurate timings..   */
    # nop

    # ldr    r3, [r0, #44] /* Fetch pointer to DWT clock  */
    # ldr    r1, [r3]      /* Capture current tick        */
    # nop                  /* Inserting nops makes sure we have a known good base value */
    # nop                  /* (One cycle would magically come and go with other instructions) */

bitCycleReceive:

    # # # Drive clock low for 4 target ticks
    # 16
    setPinLow

    ldr r2, [r0, #8]
    dwt_Wait


    # # # Wait for six target clocks before sampling
    # 16
    # Remember: Open-drain!
    setPinHigh
    ldr r2, [r0, #20]
    dwt_Wait


    # # # Sample and wait for six more target clocks
    # 22
    readPin
    lsl    r6,  #1
    orr    r6,  r2
    ldr    r2, [r0, #20]
    dwt_Wait

    sub    r7,  #1
    bne bitCycleReceive

    # Enable interrupts
    cpsie i
/* * * * * * * * * * * * * * * * * * * * * * */
    # ldr    r3, [r0, #44] /* Fetch pointer to DWT clock */
    # ldr    r2, [r3]      /* Capture new time */
    # sub    r2,  r2, r1   /* Calculate delta  */
    # sub    r2, #6        /* Subtract the time it took to capture these values */
    # str    r2, [r0, #48] /* Store result     */

    str    r6, [r0, #0 ] /* store data       */

    # Stack stuff..
    pop   {r1 - r7}
    pop   {r0}

bx r0

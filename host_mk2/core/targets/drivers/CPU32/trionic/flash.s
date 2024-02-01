# Universal bdm driver

# Known bugs / eventual problems:
# 28Fxxx:
# If format/write compare indicates a word as "not correct" only once, the optional "verify as regular data" step will be skipped.
# The chance of this happening is astronomically small (and it's optional) so I'll leave it as is atm.

# Usage:
# Setup TPURAM or SRAM to 0x100000 ( Preferably the first since it's way faster )
# Initialize CSPAR and all that stuff to base flash @ address 0 (Does not apply to MCP)
# Initialize clock
# if Trionic 5, 16,7 MHz is a must when equipped with original flash (Delays are calibrated for that)
# If it has toggle/Atmel flash just go for 20 MHz. Motorola overengineered the crap out of these so no need to chickenshit on 16 MHz ECUs
# Trionic 7 is locked to an external clock of 16 MHZ.
# Trionic 8 can be run @ 32 MHz
# Trionic 8 MCP can be set to anything, the driver will correct it to 28 MHz in the init-function.

#######
# Init:

# Upload driver to 0x100400
# Trionic 5, 7, 8 main:
# Store 3 in register D0
#
# Trionic 8 MCP:
# Store 4 in register D0
# 
# Set PC to 0x100400
# Start it

# When it enters bdm again:
# Read D0, If it's 1 everything is OK, if not 1 _ABORT_!
# Does not apply to MCP:
# D7 contains manufacturer and device ID
# D3 contains extended ID (Trionic 7 and 8)
# D6 contains which type of flash (1 Trionic 5 stock flash, 2 toggle flash, 3 Atmel)
# A1 contains size of flash (Store this value now)

# Really useful on Trionic 5 since you only have to make sure selected file is equal in size or smaller
# Set up a loop that divides size by file size to flash everything
# (You can also bump the clock here if D6 is higher than 1 on Trionic 5)

########
# Erase:

# Store 2 in register D0, (This tells the driver to format flash)
# Set PC to 0x100400
# Start it

# When it enters bdm again:
# Read D0, If it's 1 everything is OK, if not 1 _ABORT_!
# Does not apply to MCP:
# A0 contains last address that was worked on (Only useful if something went wrong and you want to know where)

########
# Write:

# If previous command went ok you already have the right value in D0
# Write 0 to A0

# -:"loop":-
# Upload 1024 bytes starting from 0x100000
# Set PC to 0x100400
# Start it

# When it enters bdm again:
# Read D0.
# If 1, repeat loop. ( A0 autoincrements and D0 already is 1 )
# If 0, something went wrong. _ABORT_




# Notes about MCP:
# The shadow region must be stored above everything in the binary file(0x40000 - 0x40100)
# As you can see it's only 256 bytes so only upload that much the last time. The driver is aware of it and won't write more than necessary
#
# There is no retry-counter so expect it to get stuck if the flash is broken.

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

    movea.l #0x1007FC, %sp /* Reset stack pointer */
    
    cmpi.b  #1, %d0
    beq.b   WriteBuffer    
    cmpi.b  #2, %d0
    beq.b   FormatFlash    
    cmpi.b  #3, %d0
    beq.b   Syscfg
    cmpi.b  #4, %d0 /* Special case: Init MCP */
    beq.w   InitMCP
    bra.b   NiceTry

WriteBuffer:

    # Store buffer location
    # Store number of WORDS to write
    movea.l #0x100000, %a1
    move.w  #512     , %d1
    
    cmpi.w  #1       , %d6
    beq.w   WriteBufferOLD
    cmpi.w  #2       , %d6
    beq.w   WriteBufferNEW
    cmpi.w  #3       , %d6
    beq.w   WriteBufferAtmel
    cmpi.w  #4       , %d6
    beq.w   WriteBufferMCP
    bra.b   NiceTry
    
FormatFlash:

    # Start from address 0
    suba.l  %a0      , %a0
    
    cmpi.w  #1       , %d6
    beq.w   FormatFlashOLD
    cmpi.w  #2       , %d6
    beq.w   FormatFlashNEW
    cmpi.w  #3       , %d6
    beq.w   FormatFlashAtmel
    cmpi.w  #4       , %d6
    beq.w   FormatFlashMCP
NiceTry:
    bsr.b   Delay
    clr.l   %d0
bgnd

Syscfg: 

    # This abomination requires further work..    
    moveq.l #1       , %d0  /* Presume result to be ok   */
    
    moveq.l #0x40    , %d1  /* Used for H/W on Trionic 5 */
    movea.l #0xFFFC14, %a0

    move.w  #0x5555  , %d4  /* Used by L/W flsh routines */
    movea.l #0xAAAA  , %a2
    movea.l #0x5554  , %a6

    suba.l  %a5      , %a5  /* Reset pointer to addr 0   */

    # Make a copy of address 0 and Send ID CMD for L/W flash
    move.w  (%a5)    , %d3
    move.w  %a2      ,(%a2)
    move.w  %d4      ,(%a6)
    move.w  #0x9090  ,(%a2)
    
    bsr.b   Delay

    # Make a new copy of address 0 and compare
    move.w  (%a5)+   , %d7
    cmp.w   %d3      , %d7
    beq.b   TryHW            /* Try H/W if same val is read */
    move.b  (%a5)    , %d7   /* Copy dev ID                 */
    move.w  (%a5)    , %d3   /* Store a full copy of addr 2 */
    move.w  %a2      ,(%a2)  /* Reset flash                 */
    move.w  %d4      ,(%a6)
    move.w  #0xF0F0  ,(%a2)
    bsr.b   Delay
    bsr.b   Delay
    bsr.b   Delay
    bsr.b   Delay
    bsr.b   Delay
    moveq.l #2       , %d6   /* Ind toggle-flash */
    bra.w   LWFlash 

Delay:
    move.w  #0x1800  , %d2
Dloop:
    dbra    %d2,     Dloop   /* %d2 becomes 0xFFFF */
rts

    # Same data read, trying H/W flash
TryHW:
    moveq.l #1       , %d6   /* Ind old flash         */
    move.w  %d1      ,(%a0)+ /* Latching up H/W       */
    or.w    %d1      ,(%a0)
    bsr.b   Delay
    move.w  %d2      ,-(%a5) /* Reset flash           */
    move.w  %d2      ,(%a5)
    bsr.b   Delay
    move.w  #0x9090  ,(%a5)  /* Send ID CMD, H/W flsh */
    move.w  (%a5)+   , %d7   /* Make a new copy n cmp */
    cmp.w   %d3      , %d7
    beq.w   UnkFlash
    move.b  (%a5)    , %d7   /* Cpy dev id / ntr read */
    clr.w   -(%a5)

# # # H/W flash # # # # # # # # #

    lea     HVT      , %a0   /* Address of id table */
    moveq.l #3       , %d3   /* 3 sizes     */
    moveq.l #2       , %d5
   
NextSize:
    moveq   #1       , %d2
HVtstL:
    cmp.b   (%a0)+   , %d7
    beq.w   ID_Match
    dbra    %d2, HVtstL
    
    lsl.w   #1       , %d5   /* double size */
    subq.b  #1       , %d3
    bne.b   NextSize
    bra.b   UnkFlash

# # # L/W flash # # # # # # # # #    
    
LWFlash:
    moveq.l #8       , %d5   /* Prepare size as 0x80000  */
    move.w  %d7      , %d1   /* Store another copy of ID */
    lsr.w   %d5      , %d1   /* Shift down manuf ID      */

# Class 29 flash  
    cmpi.b  #0x01    , %d1   /* AMD         */
    beq.b   Class29
    cmpi.b  #0x20    , %d1   /* ST          */
    beq.b   Class29 
    cmpi.b  #0x1C    , %d1   /* EON         */
    beq.b   Class29 
    cmpi.w  #0x37A4  , %d7   /* AMIC    010 */
    beq.b   Size128

# Class 39 flash
    cmpi.w  #0xDAA1  , %d7   /* Winbond 010 */
    beq.b   Size128
    cmpi.w  #0x9D1C  , %d7   /* PMC     010 */
    beq.b   Size128
    cmpi.w  #0x9D4D  , %d7   /* PMC     020 */
    beq.b   Size256
    cmpi.w  #0xBFB4  , %d7   /* SST     512 */
    beq.b   Size64
    cmpi.w  #0xBFB5  , %d7   /* SST     010 */
    beq.b   Size128
    cmpi.w  #0xBFB6  , %d7   /* SST     020 */
    beq.b   Size256
    cmpi.w  #0x0022  , %d7   /* AMD, T7/T8  */    
    beq.b   Unicorns

# Atmel
    moveq.l #3       , %d6   /* Change drv  */
    cmpi.w  #0x1F5D  , %d7   /* Atmel   512 */
    beq.b   Size64    
    cmpi.w  #0x1FD5  , %d7   /* Atmel   010 */
    beq.b   Size128
    cmpi.w  #0x1FDA  , %d7   /* Atmel   020 */
    beq.b   Size256
    bra.b   UnkFlash    

Unicorns:
    moveq.l #16      , %d5   /* 256 K = 1 M */
    cmpi.w  #0x2223  , %d3   /* Trionic 7   */
    beq.b   Size128
    cmpi.w  #0x2281  , %d3   /* Trionic 8   */
    beq.b   Size256
    bra.b   UnkFlash
Class29:
    cmpi.b  #0x21    , %d7
    beq.b   Size64
    cmpi.b  #0x20    , %d7
    beq.b   Size128
UnkFlash:
    clr.l   %d6 /* Make sure no driver is selected */
    clr.l   %d0 /* Indicate fault                  */
Size64:
    lsr.w   #1       , %d5
Size128:
    lsr.w   #1       , %d5
Size256:
ID_Match:
    swap    %d5
    movea.l %d5      , %a1
    clr.l   %d5
    subq.l  #1       , %d5
bgnd

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

# Could never get the damn erase command to work.
# We'll use Atmel's weird page write feature instead
FormatFlashAtmel:
    bsr.w   Delay
    moveq.l #1, %d0
bgnd

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

# Ugly code is an understatement...
WriteBufferAtmel:

    moveq.l #127     , %d3

WriteLoopAt:
    movea.l %a0      , %a3 
    movea.l %a1      , %a4 
    move.w  %d3      , %d0

CheckLAtW: 
    cmpm.w  (%a4)+   ,(%a3)+
    bne.b   NotIdentAtW
    dbra    %d0,      CheckLAtW

    movea.l %a3      , %a0
    movea.l %a4      , %a1

    sub.w   %d3      , %d1
    subq.w  #1       , %d1      
    beq.b   WriteAtDone
    bra.b   WriteLoopAt

NotIdentAtW:
    movea.l %a0      , %a3
    movea.l %a1      , %a4
    move.w  %d3      , %d0 

# Unlock
    move.w  %a2      ,(%a2)
    move.w  %d4      ,(%a6)
    move.w  #0xA0A0  ,(%a2)

WritePageAT:
    move.w  (%a4)+   ,(%a3)+
    dbra    %d0, WritePageAT

AtmelWaitW:                       
    move.w  (%a0)    , %d0
    cmp.w   (%a0)    , %d0
    bne.b   AtmelWaitW
    bra.b   WriteLoopAt
      
WriteAtDone:
    moveq.l #1       , %d0   
bgnd

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

FormatFlashNEW:
        
    cmp.w   (%a0)    , %d5
    beq.b   DataIdentToggle
    
    move.w  %a2      ,(%a2)
    move.w  %d4      ,(%a6)
    move.w  #0x8080  ,(%a2)

    move.w  %a2      ,(%a2)
    move.w  %d4      ,(%a6)
    move.w  #0x1010  ,(%a2)

ToggleWait:
    move.w  (%a0)    , %d2
    cmp.w   (%a0)    , %d2   
    bne.b   ToggleWait
    bra.b   FormatFlashNEW
    
DataIdentToggle:
    addq.l  #2       , %a0
    cmpa.l  %a1      , %a0      
    bcs.b   FormatFlashNEW
    moveq.l #1       , %d0
bgnd

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

WriteBufferNEW:

    cmpm.w  (%a0)+   ,(%a1)+
    beq.b   ToggleIdent      
    move.w  %a2      ,(%a2)
    move.w  %d4      ,(%a6)
    move.w  #0xA0A0  ,(%a2)
    move.w  -(%a1)   ,-(%a0)

ToggleWaitW:                       
    move.w  (%a0)    , %d0
    cmp.w   (%a0)    , %d0
    bne.b   ToggleWaitW

    # Go back for verification
    bra.b   WriteBufferNEW       

ToggleIdent:
    subq.w  #1       , %d1
    bne.b   WriteBufferNEW

    # No counter yet
    moveq   #1       , %d0
bgnd

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

WriteBufferOLD:

    moveq.l #25      , %d0

WriteLoop:
    # Is it even necessary to write this byte?
    cmpm.w  (%a0)+   ,(%a1)+
    beq.b   dataident

WriteLoopM:    
    # Send write command
    move.w  #0x4040  ,-(%a0) 
    move.w -(%a1)    ,(%a0) 
    bsr.b    Delay_10uS
    
    # Send "Write compare" command
    move.w  #0xC0C0  ,(%a0)
    bsr.b   Delay_6uS
    
    # Did it stick?
    cmpm.w  (%a0)+   ,(%a1)+
    bne.b   DecWR
    # Paranoia; Revert back to read-mode and compare once more. 
    clr.w   -(%a0)
    tst.w   -(%a1)
    bra.b   WriteLoop
DecWR:    
    # Nope. Decrement tries and try again.. if allowed
    subq.b  #1       , %d0
    bne.b   WriteLoopM
bgnd

dataident:
    subq.w  #1       , %d1
    bne.b   WriteBufferOLD
    
    moveq.l #1       , %d0
    # CAT28f plays the b*tch-game. Make sure it is in read mode..
    clr.w   (%a0)
bgnd

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
Delay_10uS:
    moveq.l #15      , %d3
us10loop:
    subq.l  #1       , %d3
    bne.b   us10loop
rts
Delay_6uS:
    moveq.l #8       , %d3
us6loop:
    subq.l  #1       , %d3
    bne.b   us6loop
rts
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
   
FormatFlashOLD:

    # Zero flash
    # Make a copy of start addr
    movea.l %a0      , %a2  
OOResTries:                 
    moveq.l #25      , %d0

OOloop:
    tst.w   (%a2)+
    beq.b   OOisOO
BoostOO:    
    move.w  #0x4040  ,-(%a2)
    clr.w   (%a2)        
    bsr.b   Delay_10uS  
    move.w  #0xC0C0  ,(%a2)
    bsr.b   Delay_6uS
    tst.w   (%a2)+
    bne.b   DecOO
    clr.w   -(%a2)
    bra.b   OOloop   
DecOO:    
    subq.b  #1       , %d0 
    beq.b   EndFF
    bra.b   BoostOO
OOisOO:
    cmpa.l  %a1      , %a2 
    bcs.b   OOResTries

    # -:Format flash:-
    move.w  #1000    , %d0 /* Maximum number of tries */
    move.w  #0x2020  , %d1

FFloop:                     
    cmp.w   (%a0)+   , %d5
    beq.b   DataisFF
    move.w  %d1      ,-(%a0)
    move.w  %d1      ,(%a0)

    move.w  #0x4240  , %d3  /* Wait for 10~ mS (10,05 ish)     */
mSloop:                     
    dbra %d3, mSloop   

    move.w  #0xA0A0  ,(%a0)
    bsr.b   Delay_6uS            
    cmp.w   (%a0)    , %d5
    bne.b   DecFF
    clr.w   (%a0)        
    bra.b   FFloop
DecFF:    
    subq.w  #1       , %d0
    beq.b   EndFF
DataisFF:           
    cmpa.l    %a1      , %a0
    bcs.b   FFloop
    moveq.l #1       , %d0
   
EndFF:
bgnd

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

.equ CMFIMCR   , 0xFFF800
.equ STPCTL1   , 0xfff808
.equ CMFICTL1  , 0xFFF80C
.equ CMFICTL2  , 0xFFF80E

InitMCP:
    move.w  #0xD084  ,(0xfffa04) /* Set clock to 28 MHz */
    movea.l #CMFIMCR , %a4
    move.w  #0x9800  ,(%a4)      /* Stop CMFI           */
    movea.l #STPCTL1 , %a5
#   movea.l #CMFICTL1, %a5       /* Store some addrs    */
    clr.l   (%a5)+               /* Base at Addr 0      *//* Will increment a5 to point @ CMFICTL1 */
    move.w  #0x1800  ,(%a4)      /* Start CMFI          */
    movea.l #CMFICTL2, %a6
    clr.l   %d5
    subq.l  #1       , %d5
    moveq.l #4       , %d6 /* Set driver to MCP     */
    moveq.l #1       , %d0
    bsr.w   DisShadow
bgnd    

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
# Write functions

WriteBufferMCP:

    bsr.b   DisShadow                  /* Ugly but effective..           */
    cmpa.l  #0x40000          , %a0
    blt.b   VerifComp
    bsr.b   EnaShadow                  /* Enable sahdow access           */
    move.w  #128              , %d1    /* Shadow only has 128 words      */
    suba.l  %a0               , %a0    /* Start writing from 0           */

    # Check if page has to be written / Has been written
VerifComp:
    move.l  %a0               , %d3    /* Which partition to enable      */
    move.w  #0x100            , %d2
    lsr.l   #8                , %d3    /* (Address >> 15)                */
    lsr.l   #7                , %d3
    lsl.w   %d3               , %d2    /* 0x100 << x                     */
    move.b  #0x32             , %d2    /* xx << 8 | 0x32                 */
VerifMC:
    move.l  %a0               , %a2    /* Backup where to write          */
    move.l  %a1               , %a3    /* Backup where to read           */
VerifShrt:
    moveq.l #64               , %d3    /* Number of bytes to compare     */
    bsr.b   swsr
PageCmpL:
    cmpm.w  (%a2)+            ,(%a3)+
    bne.b   WritePage
    subq.l  #2                , %d3
    bne.b   PageCmpL
    move.l  %a2               , %a0    /* Update where to write          */
    move.l  %a3               , %a1    /* Update where to read           */
    sub.l   #32               , %d1    /* Decrement number of words left */
    bne.b   VerifShrt
bgnd

WritePage:
    move.w  %d2               ,(%a6)   /* Start session CMFICTL2         */
    move.l  %a0               , %a2    /* Backup where to write          */
    move.l  %a1               , %a3    /* Backup where to read           */
    moveq.l #64               , %d3    /* Size of page                   */
PageFill:
    move.w  (%a3)+            ,(%a2)+
    subq.w  #2                , %d3
    bne.b   PageFill
    
WritePulse:
    ori.w   #0x0001           ,(%a6)   /* Enable high voltage            */
VppActiveW:                            /* Wait for VPP to go low         */
    bsr.b   swsr
    tst.w   (%a5)
    bmi.b   VppActiveW
    andi.w  #0xFFFE           ,(%a6)   /* Disable High voltage           */

    # Perform margain read
    move.l  %a0               , %a2    /* Backup where to write          */
    moveq.l #64               , %d3    /* Size of page                   */
MargainLW:
    tst.b   (%a2)+
    bne.b   WritePulse
    subq.b  #1                , %d3
    bne.b   MargainLW
    andi.w  #0xFFFD           ,(%a6)   /* Negate session                 */
    bra.b   VerifMC                    /* Go back for verification       */

# # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # #

swsr:
    move.b  #0x55    ,(0xFFFA27)
    move.b  #0xAA    ,(0xFFFA27)
rts

# Helper: Enable / disable shadow access
DisShadow:
    bsr.b   swsr
    andi.w  #0xDFFF           ,(%a4)
rts
EnaShadow:
    bsr.b   swsr
    ori.w   #0x2000           ,(%a4)
rts
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

MCPHardCheck:
    movea.l #0x40000          , %a0 /* Last address    */
    st.b    %d1                     /* Default to fail */
HardStart:
    suba.l  %a1               , %a1 /* Start from 0    */
HardL:
    bsr.b   swsr
    cmp.w   (%a1)+            , %d5
    bne.b   HardRet
    cmpa.l  %a0               , %a1
    blt.b   HardL

    bsr.b   EnaShadow
    movea.l #0x100            , %a0
    cmpa.w  %a0               , %a1
    beq.b   ShadowVerifed
    bra.b   HardStart

ShadowVerifed:
    clr.l   %d1
HardRet:
    bsr.b   DisShadow
rts

# # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # #

FormatFlashMCP:

    move.w  #0x223C           ,(%a5) /* Configure timings */
    bsr.b   DisShadow                /* Disable shadow    */

FormatMCPL:
    move.w  #0xFF36           ,(%a6) /* Start session     */
    move.w  %d5               ,(%a0) /* Erase interlock   */

ErasePulse:
    ori.w   #0x0001           ,(%a6) /* Enable highv      */
VppActiveE:                          /* Wait for Vpp low  */
    bsr.b   swsr
    tst.w   (%a5)
    bmi.b   VppActiveE
    andi.w  #0xFFFE           ,(%a6) /* Disable Highv     */

    bsr.b   MCPHardCheck             /* Margain verify    */
    tst.b   %d1
    bne.b   ErasePulse

    andi.w  #0xFFFD           ,(%a6) /* 0x34              */
    andi.w  #0xFFF9           ,(%a6) /* End session       */

    bsr.b   MCPHardCheck             /* Check once more   */
    tst.b   %d1
    bne.b   FormatMCPL

    moveq.l #1                , %d0  /* Finally done      */
bgnd    
    

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #


.align 4
# Table for H/V flash:
# # # # # # # # AMD , Other guys
HVT:      .byte 0x25, 0xB8 /*  64 K (T5.2) */
HV128:    .byte 0xA7, 0xB4 /* 128 K (T5.5) */
HV256:    .byte 0x2A, 0xBD /* 256 K "T5.7" */

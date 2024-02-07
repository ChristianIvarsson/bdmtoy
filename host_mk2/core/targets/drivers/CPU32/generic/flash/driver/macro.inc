
# Overall
.set opReg    , d0  /* What to do */
.set retReg   , d0  /* Returned status. 1 == ok */

# 4 - Init
.set baseAddr , a0  /* Flash base */
.set spTemp   , a1  /* Used when setting stack pointer */
.set flashType, d7  /* Driver needs to know which flash to talk to */

# 2,3 - Erase
.set eraseAddr, a0  /* Erase from / currently erased address */
.set endAddr  , a1  /* Erase up to this address */

# 1 - Write
.set writeDst , a0  /* Write from / currrently written address */
.set writeSrc , a1  /* Source data buffer address */
.set writeLen , d1  /* Number of bytes to write */

.set wrkLen   , d2  /* Internal - write counter */
.set wrkSrc   , a2  /* Internal - source pointer */


# Junk
.set tmpRegA  , d3
.set ffReg    , d4  /* Automatically set to ffff before erase */



# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
# Specific to toggle and paged flash

# Flash command offsets
.set addrA    , a3
.set addrB    , a4

# Prestored commands
.set cmdA     , d5
.set cmdB     , d6

# Regular command macro
.macro lwCMD  _CMD
    move.w   cmdA        , (addrA) /* Send 0xAAAA  to address 0xAAAA(base) */
    move.w   cmdB        , (addrB) /* Send 0x5555  to address 0x5554(base) */
    move.w   #\_CMD      , (addrA) /* Send command to address 0xAAAA(base) */
.endm

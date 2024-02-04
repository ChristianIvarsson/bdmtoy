.set opReg  , d0
.set retReg , d0

# Init
# These are ONLY valid directly after init has run.
.set flMID , d4
.set flPID , d5
.set flLen , d6




# Init, erase, write
.set dstAddr, a0
.set flsTyp , d7

# Erase
.set endAddr, a1
.set ffReg  , d4

# Write
.set wrLen  , d1
.set srcAddr, a1
.set srcBck , a3
.set lenBck , d4

# Junk
.set tmpRegA, d5
.set tmpRegB, d6



# Flash command offsets
.set addrA, a4
.set addrB, a5

# Prestored commands
.set cmdA  , d2
.set cmdB  , d3


# Regular command macro
.macro lwCMD  _CMD
    move.w   cmdA        , (addrA) /* Send 0xAAAA  to address 0xAAAA(base) */
    move.w   cmdB        , (addrB) /* Send 0x5555  to address 0x5554(base) */
    move.w   #\_CMD      , (addrA) /* Send command to address 0xAAAA(base) */
.endm

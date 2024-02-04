
# # # # # # # # # # # # # #
# Passed arguments

# Flash base address
.set fBase , a0

# # # # # # # # # # # # # #
# Returned values
.set retReg , d0

.set flMID  , d4
.set flPID  , d5
.set flLen  , d6
.set flsTyp , d7

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

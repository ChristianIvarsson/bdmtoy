#ifndef __NEXUSPRIVATE_H__
#define __NEXUSPRIVATE_H__
#include "../../../shared/insdef_ppc.h"
//////////////////////////////////////////////////////////////
/// OnCE / Class 1+ //////////////////////////////////////////

/// OnCE Command Register.
typedef struct __attribute((packed)) {
    uint8_t RS:7; // Register select. Check list below
    uint8_t EX:1; // 1 = Exit debug, 0 = Remain in wonderland
    uint8_t GO:1; // 1 = Execute instruction in IR, 0 = Poke belly button
    uint8_t RW:1; // 1 = Read, 0 = Write
    uint8_t pd:6; // Padding
} OnCE_OCMD_t;

typedef struct __attribute((packed)) {
    uint32_t WBBRLower;
    uint32_t WBBRUpper;
    uint32_t       MSR;
    uint32_t        PC;
    uint32_t        IR;
    uint32_t       CTL;
} OnCE_CPUSCR_t;

/// Container where last known CPU state should be stored
// CPUSCR = 32 bits * 6
// CTL -> IR -> PC -> MSR -> WBBRUpper -> WBBRLower
struct __attribute((packed)) {
    uint32_t WBBRLower; // Write-Back Bus lower.
    uint32_t WBBRUpper; // Write-Back Bus upper. Seldom used
    uint32_t       MSR; // Machine State Register
    uint32_t        PC; // Program Counter.
    uint32_t        IR; // Instruction (as in PowerPC instruction. You can force execution of your own instructions by means of this)
    uint32_t       CTL;
/*  struct __attribute((packed)) {
        uint8_t     PAD:2;
        uint8_t IRSTAT7: 1; // IR status bit 7. Indicates a precise external termination error status for the IR.
        uint8_t IRSTAT6: 1; // IR status bit 6. This control bit indicates a parity error status for the IR.
        uint8_t IRSTAT5: 1; // IR status bit 4. Indicates an instruction address compare 5 event status for the IR.
        uint8_t IRSTAT4: 1; // IR status bit 4. Indicates an instruction address compare 3 event status for the IR.
        uint8_t IRSTAT3: 1; // IR status bit 3. Indicates an instruction address compare 2 event status for the IR.
        uint8_t IRSTAT2: 1; // IR status bit 2. Indicates an instruction address compare 1 event status for the IR.
        uint8_t IRSTAT1: 1; // IR status bit 1. Indicates a TLB miss status for the IR.
        uint8_t IRSTAT0: 1; // IR status bit 0.This control bit indicates an ERROR termination status for the IR.
        uint8_t    FFRA: 1; // Feed forward RA operand bit.
        uint8_t   PCINV: 1; // PC and IR invalid status bit
        uint8_t  PCOFST: 4; // PC offset field.
        uint16_t INSTAT:16; // Internal state bits. Should be restored after a debug-session
    } CTL;*/
} OnCE_CPUSCR;

/// Debug status register
struct __attribute((packed)) {
    uint8_t CNT1TRG:1; // Counter 1 triggered. Set if debug counter 1 is triggered by a trigger event
    uint8_t    pad0:4;
    uint8_t    CRET:1; // Critical return debug event. Set if a critical return debug event occurred
    uint8_t   CIRPT:1; // Critical interrupt taken debug event. Set if a critical interrupt taken debug event occurred.
    uint8_t   DCNT2:1; // Debug counter 2 debug event. Set if a DCNT2 debug event occurred.
    uint8_t   DCNT1:1; // Debug counter 1 debug event. Set if a DCNT1 debug event occurred.
    uint8_t   DEVT2:1; // External debug event 2 debug event. Set if a DEVT2 debug event occurred.
    uint8_t   DEVT1:1; // External debug event 1 debug event. Set if a DEVT1 debug event occurred
    uint8_t    pad1:4;
    uint8_t     RET:1; // Return debug event. Set if a Return debug event occurred
    uint8_t   DAC2W:1; // Data address compare 2 write debug event. Set if a write-type DAC2 debug event occurred while DBCR0[DAC2] = 0b01 or DBCR0[DAC2] = 0b11.
    uint8_t   DAC2R:1; // Data address compare 2 read debug event. Set if a read-type DAC2 debug event occurred while DBCR0[DAC2] = 0b10 or DBCR0[DAC2] = 0b11.
    uint8_t   DAC1W:1; // Data address compare 1 write debug event. Set if a write-type DAC1 debug event occurred while DBCR0[DAC1] = 0b01 or DBCR0[DAC1] = 0b11.
    uint8_t   DAC1R:1; // Data address compare 1 read debug event. Set if a read-type DAC1 debug event occurred while DBCR0[DAC1] = 0b10 or DBCR0[DAC1] = 0b11.
    uint8_t    IAC4:1; // Instruction address compare 4 debug event. Set if an IAC4 debug event occurred
    uint8_t    IAC3:1; // Instruction address compare 3 debug event. Set if an IAC3 debug event occurred
    uint8_t    IAC2:1; // Instruction address compare 2 debug event. Set if an IAC2 debug event occurred
    uint8_t    IAC1:1; // Instruction address compare 1 debug event. Set if an IAC1 debug event occurred
    uint8_t    TRAP:1; // Trap taken debug event. Set if a trap taken debug event occurred
    uint8_t    IRPT:1; // Interrupt taken debug event. Set if an interrupt taken debug event occurred
    uint8_t     BRT:1; // Branch taken debug event. Set if an branch taken debug event occurred.
    uint8_t    ICMP:1; // Instruction complete debug event. Set if an instruction complete debug event occurred.
    uint8_t     MRR:2; // Most recent reset. 0 No reset, 1 hard reset.
    uint8_t     UDE:1; // Unconditional debug event
    uint8_t     IDE:1; // Imprecise debug event
} OnCE_DBSR;

// OnCE OCMD commands.
#define OnCE_CMD_JTAG_ID     0b0000010 // JTAG ID (Read only)
#define OnCE_CMD_CPUSCR      0b0010000 // CPU Scan Register
#define OnCE_CMD_NOREGSEL    0b0010001 // No register select (Bypass)
#define OnCE_CMD_OCR         0b0010010 // OnCE Control Register

#define OnCE_CMD_IAC1        0b0100000 // Instruction Address Compare N
#define OnCE_CMD_IAC2        0b0100001
#define OnCE_CMD_IAC3        0b0100010
#define OnCE_CMD_IAC4        0b0100011

#define OnCE_CMD_DAC1        0b0100100 // Data Address Compare N
#define OnCE_CMD_DAC2        0b0100101

#define OnCE_CMD_DBCNT       0b0101100 // Debug Counter Register
#define OnCE_CMD_PCFIFO      0b0101101 // Debug PCFIFO (read-only)

#define OnCE_CMD_DBSR        0b0110000 // Debug Status Register
#define OnCE_CMD_DBCR0       0b0110001 // Debug Control Register N
#define OnCE_CMD_DBCR1       0b0110010
#define OnCE_CMD_DBCR2       0b0110011
#define OnCE_CMD_DBCR3       0b0110100

#define OnCE_CMD_GPISELECT0  0b1110000 // General purpose register select N
#define OnCE_CMD_GPISELECT1  0b1110001
#define OnCE_CMD_GPISELECT2  0b1110010
#define OnCE_CMD_GPISELECT3  0b1110011
#define OnCE_CMD_GPISELECT4  0b1110100
#define OnCE_CMD_GPISELECT5  0b1110101
#define OnCE_CMD_GPISELECT6  0b1110110
#define OnCE_CMD_GPISELECT7  0b1110111
#define OnCE_CMD_GPISELECT8  0b1111000
#define OnCE_CMD_GPISELECT9  0b1111001

#define OnCE_CMD_CDACNTL     0b1111010 // Cache Debug Access Control Register
#define OnCE_CMD_CDADATA     0b1111011 // Cache Debug Access Data Register

#define OnCE_CMD_NEXUS3ACC   0b1111100 // Nexus 3 Access

#define OnCE_CMD_ENABLE_ONCE 0b1111110 // Enable OnCE (no need on mpc55xx devices)
#define OnCE_CMD_BYPASS      0b1111111 // Bypass..

/*
IDCODE              00001
ACCESS_AUX_TAP_NPC  10000
ACCESS_AUX_TAP_ONCE 10001
ACCESS_AUX_TAP_eTPU 10010
ACCESS_AUX_TAP_DMA  10011
BYPASS              11111

FACTORY DEBUG:
00101
00110
01010

*/

// OnCE Control Register.
typedef struct __attribute((packed)) {
    uint8_t    DR:1; // 1 = STOP! Enter debug.
    uint8_t   FDB:1; // Force debug
    uint8_t  WKUP:1; // Wakeup request bit
    uint8_t  pad1:5;
    uint8_t    DE:1; // Debug TLB E enable
    uint8_t    DG:1; // Debug TLB G
    uint8_t    DM:1; // Debug TLB M
    uint8_t    DI:1; // Debug TLB I
    uint8_t    DW:1; // Debug TLB W
    uint8_t  pad2:2;
    uint8_t DMDIS:1; // Debug MMU
    uint16_t    pd3;
} OnCE_OCR_t;

// Debug Control Register 0.
typedef struct __attribute((packed)) {
    uint8_t    FT:1; // Freeze timers on debug event
    uint8_t  PAD0:4;
    uint8_t  CRET:1; // Critical return debug event enable
    uint8_t CIRPT:1; // Critical interrupt taken debug event enable
    uint8_t DCNT2:1; // Debug counter 2 debug event enable
    uint8_t DCNT1:1; // Debug counter 1 debug event enable
    uint8_t DEVT2:1; // External debug event 2 enable
    uint8_t DEVT1:1; // External debug event 1 enable
    uint8_t  PAD1:4;
    uint8_t   RET:1; // Return debug event enable
    uint8_t  DAC2:2; // Data address compare 2 debug event enable
    uint8_t  DAC1:2; // Data address compare 1 debug event enable
    uint8_t  IAC4:1; // Instruction address compare 4 debug event enable
    uint8_t  IAC3:1; // Instruction address compare 3 debug event enable
    uint8_t  IAC2:1; // Instruction address compare 2 debug event enable
    uint8_t  IAC1:1; // Instruction address compare 1 debug event enable
    uint8_t  TRAP:1; // Trap taken debug event enable
    uint8_t  IRPT:1; // Interrupt taken debug event enable
    uint8_t   BRT:1; // Branch taken debug event enable.
    uint8_t  ICMP:1; // Instruction complete debug event enable
    uint8_t   RST:2; // Reset control. 0 = no function, 2 = p_resetout_b set by debug reset control. Allows external device to initiate processor reset.
    uint8_t   IDM:1; // Internal debug mode
    uint8_t   EDM:1; // External debug mode
} OnCE_DBCR0_t;

typedef struct __attribute((packed)) {
    uint8_t  RW:1; // 0 = Read, 1 = Write
    uint8_t REG:7; // Register select.
} NEXUS_t;

//////////////////////////////////////////////////////////////
/// Class 2+ /////////////////////////////////////////////////





//////////////////////////////////////////////////////////////
/// Class 3+ /////////////////////////////////////////////////

typedef struct __attribute((packed)) {
    uint8_t   DV:1; // Data valid? 1 = yes
    uint8_t  ERR:1; // Read/Write error?
    uint16_t CNT:14;// Number of accesses.
    uint8_t  pad:5;
    uint8_t  BST:1; // Burst control. 0 = disable, 1 = enable
    uint8_t   PR:2; // Priority. 0 = lowest, 3 = highest
    uint8_t  MAP:3; // Map select. 0 = primary
    uint8_t   SZ:3; // Word size. 0 = 8, 1 = 16, 2 = 32, 3 = 64
    uint8_t   RW:1; // Read/Write select. 0 = read, 1 = write
    uint8_t   AC:1; // Access control. 1 = start, 0 = stop
} NEUXS_RWCS_t;

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////


#endif
// Default cfg: 01

//           upper         lower
// cfg0:1 | censr ctrl |  ser boot   | Boot mode name                         | Int fls  | Nexus    | Ser pass
//   00   | !0x55AA    | Don't care  | Internal–Censored                      | Enabled  | Disabled | Flash
//   00   |  0x55AA    | Dont't care | Internal–Public                        | Enabled  | Enabled  | Public

//   01   | Don't care |  0x55AA     | Serial–Flash password                  | Enabled  | Disabled | Flash
//   01   | Don't care | !0x55AA     | Serial–Public password                 | Disabled | Enabled  | Public

//   10   | !0x55AA    | Don't care  | External–No Arbitration–Censored       | Disabled | Enabled  | Public
//   10   |  0x55AA    | Don't care  | External–No Arbitration–Public         | Enabled  | Enabled  | Public

//   11   | !0x55AA    | Don't care  | External–External Arbitration–Censored | Disabled | Enabled  | Public
//   11   |  0x55AA    | Don't care  | External–External Arbitration–Public   | Enabled  | Enabled  | Public



// E39:
// Upper is _NOT_ 0x55AA
// Lower _IS_ 0x55AA



// Public password (64-bit constant value of 0xFEED_FACE_CAFE_BEEF); or
// Flash password (64-bit value in the shadow row of internal flash at address 0x00FF_FDD8).

// Regular flash (3 MB)
// 0x0000_0000–0x002F_FFFF
// Shadow (1KB)
// 0x00FF_FC00–0x00FF_FFFF

// Public key:
// 0x00FF_FDD8: FEED
// 0x00FF_FDDA: FACE
// 0x00FF_FDDC: CAFE
// 0x00FF_FDDE: BEEF

// 0x00FF_FDE0: upper
// 0x00FF_FDE2: lower

// Collected info:
// censor ctrl word _IS NOT_ 0x55AA
// Serial boot word _IS_ 0x55AA

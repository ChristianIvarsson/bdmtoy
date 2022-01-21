#ifndef BDMNEWPRIV_H_
#define BDMNEWPRIV_H_
#include "../../../shared/insdef_ppc.h"

static uint16_t BDMNEW_TranslateFault(uint32_t status);

// In BDM
// Idle: low
// Set: Falling edge
// Sampled: Rising edge

// (Target shift register size is 35 bits)

// A transmission looks like this:
// (3 bits)
// 1. Host clocks out a "1" until a "0" is read from the target
// 2. Clock out header and in status bits.
// 3. Send instruction or data (7 or 32 bits).

//////////////////////////////////////////////////////////////
/// "Header" sent to target (Two bits). What are our intentions? We're cheating for speed reasons so bits are reversed
// Thirty-two-bit command registers
#define NEWBDM_CORECMD              0x00 // Instruction command
#define NEWBDM_DATACMD              0x02 // Data command
// Seven-bit command registers
#define NEWBDM_TRAPCMD              0x01 // "Send trap enable data".. (Investigate!)
#define NEWBDM_DEBUGCMD             0x03 // Send command to debug port


//////////////////////////////////////////////////////////////
/// Debug port commands (Seven bits. Extended + major opcode. (Two + five)).
// "NEWBDM_DEBUGCMD"
#define NEWBDM_CMD_NOP              0x00 // NOP

#define NEWBDM_CMD_HRR              0x01 // Request hard reset
#define NEWBDM_CMD_SRR              0x02 // Request soft reset

#define NEWBDM_CMD_STARTDL          0x63 // "Start Download procedure"
#define NEWBDM_CMD_STOPTDL          0x43 // Stop download procedure

#define NEWBDM_CMD_NEGBKPNT         0x1F // Negate breakpoint. Used by both
#define NEWBDM_CMD_ASSMASKBKPNT     0x3F // Assert maskable breakpoint
#define NEWBDM_CMD_ASSNMASKBKPNT    0x7F // Assert non-maskable breakpoint

//////////////////////////////////////////////////////////////
/// Returned status (Two bits). Bits are reversed
#define NEWBDM_STAT_DATA            0x00 //
#define NEWBDM_STAT_SEQERR          0x02 // Sequence error
#define NEWBDM_STAT_CPUINT          0x01 // Interrupt occurred in BDM!
#define NEWBDM_STAT_NULL            0x03 // Null

#endif

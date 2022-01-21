/// Global enumerations.
#ifndef __ADAPTERENUM_H__
#define __ADAPTERENUM_H__

/// Command return codes
enum ReturnCodes
{
  RET_OK         = 0x0000, // Command completed successfully

  RET_BUSY       = 0x1000,

  // Requests
  RET_NOTSUP     = 0xF000, // Does not support this feature: Adapter/Interface/sub-feature/etc does not support this command.
  RET_NOTINS     = 0xF001, // Tried to access a function that has not had its pointer installed.
  RET_MALFORMED  = 0xF002, // Request is malformed in one way or another.
  RET_MIXEDQUEUE = 0xF003, // Don't mix different commands in queue
  
  // Target errors
  RET_NOTREADY   = 0xF010, // Unable to prepare target
  RET_NOSTART    = 0xF011, // Target did not start
  RET_DRIVERFAIL = 0xF012, // Driver indicated a fault

  // IO
  RET_BUSTERMERR = 0xF020, // Bus terminated error, data invalid
  RET_ILLCOMMAND = 0xF021, // Illegal command
  RET_UNKERROR   = 0xF022,
  RET_MAXRETRY   = 0xF023,

  // Memory
  RET_UNALIGNED  = 0xF040, // 68K goes wonkers if other than bytes are written/read to/from an odd addresses.
  RET_RESTRICMEM = 0xF041, // Currently only NEXUS3 can set this flag. It outright refused to grant access to requested address
  RET_GENERICERR = 0xF042, // Also NEXUS3 exclusive. Module flagged a fault
  RET_RWERR      = 0xF043,
  
  // Internal
  RET_FRAMEDROP  = 0xFF00, // Framecounter out of sync
  RET_NOPTR      = 0xFF01, // 
  RET_BOUNDS     = 0xFF02, // Tried to access an index outside of supported ECUs
  RET_NODRIVER   = 0xFF03, // No driver available for current task
  RET_OVERFLOW   = 0xFF04, // Tried to queue more commands than target can receive
  RET_MALLOC     = 0xFF05, // Could not allocate buffer on host
  
  RET_TIMEOUT    = 0xFFFE, // ..
  RET_ABANDON    = 0xFFFF, // Host-induced abort error
};

enum SecondReturn
{
  RET_TARGETRUNNING   = 0x0000,
  RET_TARGETSTOPPED   = 0x0001,
};

/// Main commands
enum MainCommands
{
  DO_ABANDON     = 0x0000, // Abort whatever you're doing, I'm tired of waiting!
  DO_TAPCMD      = 0x0001, // TAP command. Sub parameters
};

///////////////////// Sub-commands for TAP ////////////////////////

enum Master_Commands
{
  /////////////////////////////////
  /// Category: 0x00XX TAP (BDM, JTAG etc)
  
  /// Configuration
  TAP_DO_SETINTERFACE   = 0x0001,
  
  TAP_DO_PORTRESET      = 0x0010,
  TAP_DO_TARGETINITPORT = 0x0011,
  TAP_DO_TARGETREADY    = 0x0012,
  TAP_DO_TARGETRESET    = 0x0013,
  TAP_DO_TARGETSTART    = 0x0014,
  TAP_DO_TARGETSTOP     = 0x0015,
  TAP_DO_TARGETSTATUS   = 0x0016,

  /// Memory
  TAP_DO_READMEMORY     = 0x0040, // Read memory. It has slight dump-capabilities but it's not meant for it! Use DUMPMEM
  TAP_DO_WRITEMEMORY    = 0x0041, // Write memory. -||- but FILLMEM

  TAP_DO_DUMPMEM        = 0x0050, // ..
  TAP_DO_FILLMEM        = 0x0051, // ..


  /// Register
  TAP_DO_READREGISTER   = 0x0060, // Read register. Selected TAP determines what each number means
  TAP_DO_WRITEREGISTER  = 0x0061, // Write register. Selected TAP determines what each number means

  // TAP_DO_ANDMEMORY      = 0xFFFF, // Read memory, AND with sent data and write result back to memory
  // TAP_DO_ORMEMORY       = 0xFFFF, // Read memory, OR with sent data and write result back to memory
  // TAP_DO_XORMEMORY      = 0xFFFF, // Read memory, XOR with sent data and write result back to memory




  // 
  TAP_DO_ASSISTFLASH    = 0x0052, // host -> adapter, request
  TAP_DO_ASSISTFLASH_IN = 0x0152, // host <-> adapter, ongoing
  
  
  TAP_DO_ExecuteIns     = 0x0070, // Execute instruction
  TAP_DO_ReleaseTarg    = 0x0071, // Reset target and restore pins
  
  
  TAP_DO_UPDATESTATUS   = 0x0100, // adapter -> host
  
  
  /////////////////////////////////
  /// Category: 0x01XX Future
};

// TAP_DO_SETINTERFACE, TYPE
enum TAP_IO {
  TAP_IO_BDMOLD        = 0x0000, // Oldschool 17-bit BDM.
  TAP_IO_BDMNEW        = 0x0001, // Newschool, abomination McThing, BDM.
  TAP_IO_BDMS          = 0x0003, // Single-wire BDM. HCS12 (etc..?)
  TAP_IO_UARTMON       = 0x0004, // Weird-ass protocol used by some 68hc08's
  TAP_IO_JTAG          = 0x0010, // JTAG. Not much supported atm

  TAP_IO_NEXUS1        = 0x0021, // Freescale's OnCE / NEXUS 1 class interface
  TAP_IO_NEXUS2        = 0x0022, // NEXUS 2 class interface (not implemented)
  TAP_IO_NEXUS3        = 0x0023, // NEXUS 3 class interface
};

// TAP_DO_SETINTERFACE, SPEED
enum TAP_SPEED {
  TAP_SPEED_SLOW       = 0x0000, // Turd-gear engaged. Slowest possible software transfer.
  TAP_SPEED_MEDIUM     = 0x0001, // Slightly faster but still bitbanged.
  TAP_SPEED_FAST       = 0x0002, // Fastest possible bitbanging.
  TAP_SPEED_CUSTOM     = 0x0003, // Host-defined clock rate.

  TAP_SPEED_0_75MHZ    = 0x0010,
  TAP_SPEED_1_5MHZ     = 0x0011,
  TAP_SPEED_3MHZ       = 0x0012,
  TAP_SPEED_6MHZ       = 0x0013,
  TAP_SPEED_12MHZ      = 0x0014,
};

/// Part of TAP_DO_SETINTERFACE, cfgmask
enum endians 
{
  TAP_BIGENDIAN    = 1,
  TAP_LITTLEENDIAN = 0
};

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Host specific

enum storagetypes {
  type_flash = 0,
  type_eeprm = 1
};

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Register specification

enum NEXUS_Regs {

  // Hackies
  JTAG_IREG               = 0x0000,
  JTAG_DREG               = 0x0001,

  // Misc
  NEXUS_JTAGID            = 0x0002, // This may or may not have the same id as the standard JTAG request (Freescale...)
  NEXUS_OSR               = 0x0003,
  NEXUS_OCMD              = 0x0004,

  // CPUSCR
  NEXUS_CPUSCR            = 0x0010,
  NEXUS_CPUSCR_CTL        = 0x0011,
  NEXUS_CPUSCR_IR         = 0x0012,
  NEXUS_CPUSCR_PC         = 0x0013,
  NEXUS_CPUSCR_MSR        = 0x0014,
  NEXUS_CPUSCR_WBBRUpper  = 0x0015,
  NEXUS_CPUSCR_WBBRLower  = 0x0016,
};

enum SPI_PHASE    { SPI_FIRST = 0, SPI_SECOND = 1 };
enum SPI_POLARITY { SPI_LOW   = 0, SPI_HIGH   = 1 };
enum SPI_ORDER    { SPI_LSB   = 0, SPI_MSB    = 1 };
enum SPI_SIZE     { SPI_BYTE  = 0, SPI_WORD   = 1 };

#endif

#ifndef __CORE_H
#define __CORE_H
#ifdef __cplusplus 
extern "C" {
#endif

#include <stdint.h>

#include "../../shared/enums.h"
#include "../../shared/cmddesc.h"

#include "core_debug.h"

#include "utils/utils.h"
#include "targets/checksum/checksum.h"


#define CoreVersion "mcGizmo v00.00.00.0"

typedef void     usbSnd_t (void *ptr, uint32_t noBytes);

typedef uint32_t fnDump_t (uint32_t address, uint32_t length);
typedef uint32_t fnWrte_t (uint32_t address, uint32_t length, const void *data);
typedef uint32_t fnGen_t  (void);

// Last resort solution for now..
extern const uint32_t numberoftargets_;

// these are just templates. Actual parameters can be found in "target_descriptors.h"
extern const void *supportedTargets[];

typedef const struct 
{
    // Code pointers
    const void     *initcode; // In case something must be done to the target before starting debug
    
    // Misc..
    const uint32_t  rowItems;
    const void     *regRow;   // Row of register names
} debug_td;

typedef const struct 
{
    // Flash parameters
    const uint32_t FLASH_Address;  // Where is it located?
    const uint32_t FLASH_NoBytes;  // How much storage does it have?

    // Eeprom parameters
    const uint32_t EEPROM_Address;  // Where is it located?
    const uint32_t EEPROM_NoBytes;  // How much storage does it have?

    // SRAM parameters
    const uint32_t SRAM_Address;   // Where is it located?
    const uint32_t SRAM_NoBytes;   // How much SRAM does it have?

    // Core parameters
    fnGen_t  *const initcode;      // Which function to jump to to initialize the target
    // FLASH
    fnWrte_t *const writeFlash;    // Which function to use for flashing
    fnDump_t *const dumpFlash;     // Which function to use for dumping
    // EEPROM
    fnWrte_t *const writeEep;      // Which function to use for writing
    fnDump_t *const dumpEep;       // Which function to use for reading
    // SRAM
    fnWrte_t *const writeSram;     // Which function to use for writing
    fnDump_t *const dumpSram;      // Which function to use for reading

    // Debug pointer
    debug_td       *debug;         // Which STRUCTURE to fetch info from
    
    // TAP parameters
    const uint16_t TAP_drive;      // Which TAP interface to use
    const uint32_t TAP_frequency;  // Frequency of said interface
    
    // Human junk
    const char     *name[1];        // Target name (Pointer, ignore the size. Just don't use memcpy or some stupid stuff)
    const char     *info[1];        // Target type (Pointer, ignore the size. Just don't use memcpy or some stupid stuff)
} trgTemplate;

// Core, setup/init/target info
const char    *core_VersionString();                        // Return version string
      void     core_InstallCallback (const void *funcptr);  // Worker must know where to jump to in case it wants something.
      void     core_InstallProgress (const void *funcptr);  // Of less concern. Forward progress to this pointer
      void     core_InstallMessage  (      void *funcptr);  // Of less concern.
      void     core_InstallSendArray(usbSnd_t   *funcptr);  //
      void     core_castText        (const char *str,...);  // If possible, forward text messages to host app.
      uint32_t core_NoTargets();                            // Call this one before any of the preceding commands.
      void     core_PrintInfo       (const uint32_t index);
const char    *core_TargetName      (const uint32_t index); // Return human readable name of selected target
const char    *core_TargetInfo      (const uint32_t index); // ...
      uint32_t core_TargetSizeFLASH (const uint32_t index); // Return FLASH size
      uint32_t core_TargetSizeSRAM  (const uint32_t index); // Return SRAM size
      uint32_t core_TargetSizeEEPROM(const uint32_t index);
      uint32_t core_TargetHasDebug  (const uint32_t index);
      void     core_DumpFLASH       (const uint32_t index);
      void     core_DumpEEPROM      (const uint32_t index);        
      void     core_DumpSRAM        (const uint32_t index);
      void     core_FLASH           (const uint32_t index, const void *data);
      void     core_WriteEEPROM     (const uint32_t index, const void *data);
      void     core_WriteSRAM       (const uint32_t index, const void *data);
      void     core_WriteSRAMcust   (const uint32_t index, const void *data, const uint32_t address, const uint32_t len);
// Worker functions:
      void     core_HandleRecData  (const void *bufin, uint32_t recLen);
      uint32_t core_ReturnWorkStatus();                     // Return 1 if work completed successfully
      uint32_t core_ReturnFaultStatus();                    // Return > 0 if any faults are stored
      char    *core_TranslateFault();                       // Return a human readable string of what went wrong
const void    *core_ReturnBufferPointer();                  // Return a pointer to internal filebuffer. It _WILL_ return 0 if something is wrong so check for null pointer!
      uint32_t core_ReturnNoBytesInBuffer();                // Return number of bytes in buffer
      void     waitms(uint32_t ms);
#ifdef __cplusplus 
}
#endif
#endif

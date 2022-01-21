#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>

#include "core.h"
#include "targets/target_descriptors.h"

// SHUT UP!
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wunused-parameter"

// Last resort solution for now..
const uint32_t numberoftargets_ = numberoftargets;

// Return version string of core
const char *core_VersionString()
{   return CoreVersion;       }

// Return number of supported targets. Target list starts @ index 1
uint32_t core_NoTargets()
{   return numberoftargets;   }

void core_PrintInfo(const uint32_t index)
{
    uint32_t eeprom, flash, sram;

    if (index > numberoftargets || index == 0) {
        core_castText("Index out of bounds");
        return;
    }

    eeprom = core_TargetSizeEEPROM(index);
    flash = core_TargetSizeFLASH(index);
    sram = core_TargetSizeSRAM(index);

    if (eeprom)
        core_castText("Target size (EEPROM): %u Bytes", eeprom);
    if (flash)
        core_castText("Target size (FLASH) : %u Bytes", flash);
    if (sram)
        core_castText("Target size (SRAM)  : %u Bytes", sram);

    core_castText("Target info         : %s", core_TargetInfo(index));
}

// Return human understandable name of selected target
const char *core_TargetName(const uint32_t index)
{
    trgTemplate *targetstruc = (trgTemplate *) supportedTargets[index];
    
    if (index > numberoftargets || index == 0)
    {   return "Out of bounds";              }

    return (const char *) *targetstruc->name;
}

// Return human understandable info about the target
const char *core_TargetInfo(const uint32_t index)
{
    trgTemplate *targetstruc = (trgTemplate *) supportedTargets[index];
    
    if (index > numberoftargets || index == 0)
    {   return "Out of bounds";              }

    return (const char *)*targetstruc->info;
}

// Return size of FLASH
uint32_t core_TargetSizeFLASH(const uint32_t index)
{
    trgTemplate *targetstruc = (trgTemplate *) supportedTargets[index];
    
    if (index > numberoftargets || index == 0)
    {   return 0;                            }

    return targetstruc->FLASH_NoBytes;
}

// Return size of EEPROM
uint32_t core_TargetSizeEEPROM(const uint32_t index)
{
    trgTemplate *targetstruc = (trgTemplate *) supportedTargets[index];
    
    if (index > numberoftargets || index == 0)
    {   return 0;                            }

    return targetstruc->EEPROM_NoBytes;
}

// Return size of SRAM
uint32_t core_TargetSizeSRAM(const uint32_t index)
{
    trgTemplate *targetstruc = (trgTemplate *) supportedTargets[index];
    
    if (index > numberoftargets || index == 0)
    {   return 0;                            }

    return targetstruc->SRAM_NoBytes;
}

uint32_t core_TargetHasDebug(const uint32_t index)
{
    trgTemplate *targetstruc = (trgTemplate *) supportedTargets[index];
    
    if (index > numberoftargets || index == 0)
    {   return 0;                            }

    return targetstruc->debug ? 1 : 0;
}

char *core_TranslateFault()
{
    uint32_t Fault = core_ReturnFaultStatus();
    static char temp[64];

    switch (Fault)
    {
        case RET_TIMEOUT:
            return "Adapter took too long to do something";
        case RET_OK:
            return "No flagged faults";
            
            // These can also be thrown if you try to access TAP commands without first telling it which mode it should be in
        case RET_NOTSUP:
        case RET_NOTINS:
            return "Current adapter firmware does not support requested function or internal bug";
        
        case RET_MALFORMED:
        case RET_FRAMEDROP:
            return "Communication error";

        case RET_UNALIGNED:
            return "Unaligned access error";
        case RET_RESTRICMEM:
            return "Target refused access to requested address";
        case RET_GENERICERR:
            return "Target threw a generic fault (Blame NEXUS for this generic error message)";
        case RET_OVERFLOW:
            return "Internal bug: Code tried to overflow adapter!";
            
        default:
            // Whine whine whine... Not switching! Microbob's implementation doesn't work in *nix
            sprintf(temp, "Could not translate fault code. (0x%04X)", Fault);
            return temp;
    }

    return "Stupid compiler...";
}

typedef void castmessage(char *text);
static void *msgptr = 0;

void core_castText(const char *str, ...)
{
    char tmp[256];
    castmessage *cast = (castmessage *) msgptr;
    if (msgptr)
    {
        va_list ap;
        va_start(ap, str);
        if (vsprintf(tmp, str, ap) > 0)
            cast(tmp);
        va_end(ap);
    }
}

void core_InstallMessage(void *funcptr)
{   msgptr = (void *)funcptr;               }

// Microbob had no "sleep" function (nor does that function have great granularity).
void waitms(uint32_t ms)
{
    clock_t trg = ms * (CLOCKS_PER_SEC/1000);
    clock_t old = clock();
    clock_t dlt;
    
    // Do it this way to make sure we don't crap out during a rollover
    do{ dlt = (clock_t) (clock() - old);
    } while (dlt < trg);
}

// Ah.. the silence was nice. Let's restore the whine
#pragma GCC diagnostic pop

#ifdef __cplusplus 
}
#endif

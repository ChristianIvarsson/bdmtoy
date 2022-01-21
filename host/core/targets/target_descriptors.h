#ifndef __TARGETDESCRIPTORS_H
#define __TARGETDESCRIPTORS_H
#ifdef __cplusplus 
extern "C" {
#endif
#include "../core_worker.h"
#include "targets.h"
#include "../debug/dbg_hcs12.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
/* Notes:
 * If you don't need a certain function, just insert a 0. Code will check such things and just skip ahead.
 *
 *
 */

#define numberoftargets     ((sizeof(supportedTargets) / sizeof(void*)) - 1)

/////////////////
// CPU32 targets
trgTemplate Target_Trionic52 =
{ 0x000000, 0x020000,        // FLASH info  (Start, Length)
  0       , 0       ,        // EEPROM info (Start, Length)
  0x100000, 0x008000,        // SRAM info   (Start, Length)
  &initTrionic5     ,
  &flashTrionic     , &dumpGenericBE4,
  0                 , 0       ,
  0                 , 0       ,
  0                 ,
  TAP_IO_BDMOLD, TAP_SPEED_6MHZ
  , "Trionic 5.2"
  , "MC68332, CPU32"
};

trgTemplate Target_Trionic55 = 
{ 0x000000, 0x040000,        // FLASH info  (Start, Length)
  0       , 0       ,        // EEPROM info (Start, Length)
  0x100000, 0x008000,        // SRAM info   (Start, Length)
  &initTrionic5     ,
  &flashTrionic     , &dumpGenericBE4,
  0                 , 0       ,
  0                 , 0       ,
  0                 ,
  TAP_IO_BDMOLD, TAP_SPEED_6MHZ
  , "Trionic 5.5"
  , "MC68332, CPU32"
};

// Why not? Hardware can take it :D
trgTemplate Target_Trionic57 = 
{ 0x000000, 0x080000,        // FLASH info  (Start, Length)
  0       , 0       ,        // EEPROM info (Start, Length)
  0x100000, 0x008000,        // SRAM info   (Start, Length)
  &initTrionic5     ,
  &flashTrionic     , &dumpGenericBE4,
  0                 , 0       ,
  0                 , 0       ,
  0                 ,
  TAP_IO_BDMOLD, TAP_SPEED_6MHZ
  , "Trionic 5 512K (You don't have this)"
  , "MC68332, CPU32 (With a whopping 512K of FLASH!)"
};

trgTemplate Target_Trionic7 =
{ 0x000000, 0x080000,        // FLASH info  (Start, Length)
  0       , 0       ,        // EEPROM info (Start, Length)
  0x100000, 0x010000,        // SRAM info   (Start, Length)
  &initTrionic5     ,
  &flashTrionic     , &dumpGenericBE4,
  0                 , 0       ,
  0                 , 0       ,
  0                 ,
  TAP_IO_BDMOLD, TAP_SPEED_6MHZ
  , "Trionic 7"
  , "MC68332 derivative, CPU32"
};

trgTemplate Target_Trionic8_main =
{ 0x000000, 0x100000,        // FLASH info  (Start, Length)
  0       , 0       ,        // EEPROM info (Start, Length)
  0x100000, 0x008000,        // SRAM info   (Start, Length)
  &initT8main       ,
  &flashTrionic     , &dumpGenericBE4,
  0                 , 0       ,
  0                 , 0       ,
  0                 ,
  TAP_IO_BDMOLD, TAP_SPEED_12MHZ
  , "Trionic 8; Main"
  , "MC68377, CPU32x"
};

trgTemplate Target_Trionic8_mcp =
{ 0x000000, 0x040100,        // FLASH info  (Start, Length)
  0       , 0       ,        // EEPROM info (Start, Length)
  0x100000, 0x001800,        // SRAM info   (Start, Length)
  &initT8mcp        ,
  &flashMCP         , &dumpMCP,
  0                 , 0       ,
  0                 , 0       ,
  0                 ,
  TAP_IO_BDMOLD, TAP_SPEED_12MHZ
  , "Trionic 8; MCP"
  , "MC68F375, CPU32"
};

/////////////////
// HC12 targets
trgTemplate Target_SID_MY0405 =
{ 0x000000, 0x040000,        // FLASH info  (Start, Length)
  0x004000, 0x001000,        // EEPROM info (Start, Length)
  0x001000, 0x003000,        // SRAM info   (Start, Length)
  &initSID95_MY0405 ,
  &flashSID95       , &dumpSID95,
  &writeeepSID95    , &readeepSID95,
  0                 , &dumpGenericBE2,
  0                 ,
  // &debghcs12        ,
  TAP_IO_BDMS, TAP_SPEED_CUSTOM
  , "9-5 SID, MY04/05"
  , "MC9S12DJ256B, HCS12/CPU12"
};

trgTemplate Target_SIU_SAAB95 =
{ 0x000000, 0x020000,        // FLASH info  (Start, Length)
  0x001000, 0x000800,        // EEPROM info (Start, Length)
  0x002000, 0x002000,        // SRAM info   (Start, Length)
  &initSIU95        ,
  &flashSID95       , &dumpSID95,
  &writeeepSID95    , &readeepSID95,
  0                 , 0       ,
  0                 ,
  TAP_IO_BDMS, TAP_SPEED_CUSTOM
  , "9-5 SIU, MY06+"
  , "MC9S12DG128, HCS12/CPU12"
};

trgTemplate Target_BMW_CLUSTERUNK =
{ 0x000000, 0x020000,        // FLASH info  (Start, Length)
  0       , 0       ,        // EEPROM info (Start, Length)
  0       , 0       ,        // SRAM info   (Start, Length)
  &initBMWcluster   ,
  0                 , &dumpBMWcluster,
  0                 , 0       ,
  0                 , 0       ,
  0                 ,
  TAP_IO_BDMS, TAP_SPEED_CUSTOM
  , "BMW cluster, heck do I know"
  , "XC912DG128CPV8, HCS12/CPU12"
};

/////////////////
// NEXUS targets
trgTemplate Target_MPC5566 =
{ 0         , 0x080000,        // FLASH info  (Start, Length)
  0         , 0       ,        // EEPROM info (Start, Length)
  0x40000000, 0x020000,        // SRAM info   (Start, Length)
  &initMPC5566      ,
  0                 , &nexusDump,
  0                 , 0       ,
  0                 , &dumpSRAM_mpc5566,
  0                 ,
  TAP_IO_JTAG, TAP_SPEED_CUSTOM
  , "MPC5566, heck do I know"
  , "MPC5566, PowerPC"
};

/////////////////
// Newschool BDM targets
trgTemplate Target_EDC16C39 =
{ 0x000000, 
  
  // 0x41000,
  //0x11000,
  0x200000,        // FLASH info  (Start, Length)
  0       , 0x001000,        // EEPROM info (Start, Length)
  0x3F8000, 0x008000,        // SRAM info   (Start, Length)
  &initEDC16C39     ,
  &flashC39         , &dumpGenericBE2, // Adapter quirk. We're using DMA while dumping / filling so the words are already flipped
  &writeeepC39      , &readeepC39,
  0                 , &dumpGenericBE2,
  0                 ,
  TAP_IO_BDMNEW, TAP_SPEED_CUSTOM
  , "EDC16C39"
  , "Write me.."
};

/////////////////
// uart monitor targets (68hc08)
trgTemplate TargetE39MPU =
{ 0x00ee00,     4096,        // FLASH info  (Start, Length)
  0       , 0       ,        // EEPROM info (Start, Length)
  0x000080,     4096,        // SRAM info   (Start, Length)
  &initUARTMON,
  0                 , &dumpUARTMON,
  0                 , 0       ,
  0                 , 0       ,
  0                 ,
  TAP_IO_BDMS, TAP_SPEED_CUSTOM
  , "ACDelco e39 mpu"
  , "sqc6mdte, 68HC08/CPU08"
};




const void *supportedTargets[] = 
{
    0, // Dummy, do not assign location 0
    
    // CPU32 targets
    &Target_Trionic52,
    &Target_Trionic55,
    &Target_Trionic57,
    &Target_Trionic7,
    &Target_Trionic8_main,
    &Target_Trionic8_mcp,
    
    // HC12 targets
    &Target_SID_MY0405,
    &Target_SIU_SAAB95,
    &Target_BMW_CLUSTERUNK,

    // NEXUS targets
    &Target_MPC5566,
    
    // Newschool BDM targets
    &Target_EDC16C39,
    
    
    // uart monitor targets. (68hc08)
    &TargetE39MPU,

};

// Ah.. the silence was nice. Let's restore the whine
#pragma GCC diagnostic pop

#ifdef __cplusplus 
}
#endif
#endif

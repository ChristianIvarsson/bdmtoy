#ifndef CORE_DEBUG
#define CORE_DEBUG
#ifdef __cplusplus 
extern "C" {
#endif
#include "core.h"

const void    *debug_RegisterList   (const uint32_t index);
      uint32_t debug_noRegisters    (const uint32_t index);

#ifdef __cplusplus 
}
#endif
#endif

#ifndef __NEXUS_H__
#define __NEXUS_H__

void NEXUS_setup         (const float TargetFreq, const uint16_t prescaler, const uint8_t generation);
void NEXUS_TargetReady   (const uint16_t *in, uint16_t *out); // Reset tap to a known state

void NEXUS_ReadMemory    (const uint16_t *in, uint16_t *out);
void NEXUS_WriteMemory   (const uint16_t *in, uint16_t *out);

void NEXUS_ReadRegister  (const uint16_t *in, uint16_t *out);
void NEXUS_WriteRegister (const uint16_t *in, uint16_t *out);

void NEXUS_ExecuteIns    (const uint16_t *in, uint16_t *out);

#endif

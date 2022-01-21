#ifndef __BDMSH_H__
#define __BDMSH_H__

void BDMHCS12_setup          (const float TargetFreq);
void BDMHCS12_InitPort       (const uint16_t *in, uint16_t *out);

void BDMHCS12_TargetReady    (const uint16_t *in, uint16_t *out);
void BDMHCS12_TargetReset    (const uint16_t *in, uint16_t *out);
void BDMHCS12_TargetStart    (const uint16_t *in, uint16_t *out);

void BDMHCS12_WriteMemory    (const uint16_t *in, uint16_t *out);
void BDMHCS12_ReadMemory     (const uint16_t *in, uint16_t *out);
void BDMHCS12_FillMemory     (const uint16_t *in, uint16_t *out);
void BDMHCS12_DumpMemory     (const uint16_t *in, uint16_t *out);

void BDMHCS12_WriteRegister  (const uint16_t *in, uint16_t *out);
void BDMHCS12_ReadRegister   (const uint16_t *in, uint16_t *out);

void BDMHCS12_ReleaseTarg    (const uint16_t *in, uint16_t *out);
#endif

#ifndef BDMOLD_H_
#define BDMOLD_H_

void BDMOLD_setup         (const float TargetFreq, const uint16_t prescaler);
void BDMOLD_InitPort      (const uint16_t *in, uint16_t *out);

void BDMOLD_TargetReady   (const uint16_t *in, uint16_t *out);
void BDMOLD_TargetReset   (const uint16_t *in, uint16_t *out);
void BDMOLD_TargetStart   (const uint16_t *in, uint16_t *out);
void BDMOLD_TargetStatus  (const uint16_t *in, uint16_t *out);

void BDMOLD_WriteMemory   (const uint16_t *in, uint16_t *out);
void BDMOLD_ReadMemory    (const uint16_t *in, uint16_t *out);
void BDMOLD_FillMemory    (const uint16_t *in, uint16_t *out);
void BDMOLD_DumpMemory    (const uint16_t *in, uint16_t *out);

void BDMOLD_WriteRegister (const uint16_t *in, uint16_t *out);
void BDMOLD_ReadRegister  (const uint16_t *in, uint16_t *out);

void BDMOLD_AssistFlash   (const uint16_t *in, uint16_t *out);

#endif

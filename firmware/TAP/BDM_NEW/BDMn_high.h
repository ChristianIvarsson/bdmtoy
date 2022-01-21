#ifndef BDMNEW_H_
#define BDMNEW_H_


void BDMNEW_setup         (const float TargetFreq, const uint16_t prescaler);
void BDMNEW_InitPort      (const uint16_t *in, uint16_t *out);

// void BDMNEW_TargetReady   (const uint16_t *in, uint16_t *out);
void BDMNEW_TargetReset   (const uint16_t *in, uint16_t *out);
void BDMNEW_TargetStart   (const uint16_t *in, uint16_t *out);
void BDMNEW_TargetStatus  (const uint16_t *in, uint16_t *out);

void BDMNEW_WriteMemory   (const uint16_t *in, uint16_t *out);
void BDMNEW_ReadMemory    (const uint16_t *in, uint16_t *out);
void BDMNEW_FillMemory    (const uint16_t *in, uint16_t *out);
void BDMNEW_DumpMemory    (const uint16_t *in, uint16_t *out);

void BDMNEW_WriteRegister (const uint16_t *in, uint16_t *out);
void BDMNEW_ReadRegister  (const uint16_t *in, uint16_t *out);

void BDMNEW_ExecuteIns    (const uint16_t *in, uint16_t *out);
void BDMNEW_AssistFlash   (const uint16_t *in, uint16_t *out);

void BDMNEW_ReleaseTarg   (const uint16_t *in, uint16_t *out);

void MPCBDMTEST();



#endif

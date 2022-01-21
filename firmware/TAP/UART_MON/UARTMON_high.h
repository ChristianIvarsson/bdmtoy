



void UARTMON_setup         (const uint32_t TargetFreq);
void UARTMON_InitPort      (const uint16_t *in, uint16_t *out);

// void UARTMON_TargetReady   (const uint16_t *in, uint16_t *out);
void UARTMON_TargetReset   (const uint16_t *in, uint16_t *out);
// void UARTMON_TargetStart   (const uint16_t *in, uint16_t *out);
// void UARTMON_TargetStatus  (const uint16_t *in, uint16_t *out);

void UARTMON_WriteMemory   (const uint16_t *in, uint16_t *out);
void UARTMON_ReadMemory    (const uint16_t *in, uint16_t *out);
// void UARTMON_FillMemory    (const uint16_t *in, uint16_t *out);
// void UARTMON_DumpMemory    (const uint16_t *in, uint16_t *out);

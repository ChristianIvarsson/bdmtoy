#ifndef __JTAG__
#define __JTAG__

void JTAG_setup         (const float TargetFreq, const uint16_t prescaler);
void JTAG_InitPort      (const uint16_t *in, uint16_t *out);

void JTAG_TargetReset   (const uint16_t *in, uint16_t *out); // Target reset / set pins to the correct state
void JTAG_TargetReady   (const uint16_t *in, uint16_t *out); // Reset tap to a known state

void JTAG_WriteRegister (const uint16_t *in, uint16_t *out);
void JTAG_ReadRegister  (const uint16_t *in, uint16_t *out);

#endif

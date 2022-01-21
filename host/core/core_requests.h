#ifndef __REQUESTS_H
#define __REQUESTS_H
#ifdef __cplusplus 
extern "C" {
#endif

/// Critical Commands
void *TAP_SetInterface(TAP_Config_host_t config); // Must be called first of all!
void *TAP_PortReset();      // Reset _ALL_ TAP pins to safe values
void *TAP_TargetInitPort(); // Set pins to preferred values for target type
void *TAP_TargetReady();    // Do what is necessary to make the target accept commands.
void *TAP_TargetReset();    // Reset target. No more, no less
void *TAP_TargetStart();    // Do what is necessary to start the target
void *TAP_TargetStop();     // Do what is necessary to stop the target. On some targets this is the same as ready, on others an interrupt is sent
void *TAP_TargetRelease();

void *TAP_ReadByte   (uint32_t Addr);
void *TAP_ReadWord   (uint32_t Addr);
void *TAP_ReadDword  (uint32_t Addr);
void *TAP_ReadArr    (uint32_t Addr, uint32_t Len); // Limited dump, Use care not to overrun adapter buffer

void *TAP_WriteByte  (uint32_t Addr, uint8_t     data);
void *TAP_WriteWord  (uint32_t Addr, uint16_t    data);
void *TAP_WriteDword (uint32_t Addr, uint32_t    data);

void *TAP_Dump       (uint32_t Addr, uint32_t Len); // Sends directly to the file buffer
void *TAP_Fill       (uint32_t Addr, uint32_t Len); // Fills directly from the file buffer
void *TAP_AssistFlash(uint32_t Addr, uint32_t Len, uint32_t DriverStart, uint32_t BufferStart, uint32_t BufferLen);

/// Raw register commands
void *TAP_ReadRegByte   (uint16_t Reg               );
void *TAP_ReadRegWord   (uint16_t Reg               );
void *TAP_ReadRegDword  (uint16_t Reg               );
void *TAP_ReadRegister  (uint16_t Reg, uint16_t Size);
void *TAP_WriteRegByte  (uint16_t Reg, uint8_t  Data);
void *TAP_WriteRegWord  (uint16_t Reg, uint16_t Data);
void *TAP_WriteRegDword (uint16_t Reg, uint32_t Data);
void *TAP_WriteRegister (uint16_t Reg, uint16_t Size, uint32_t data);

// Weird commands, it'll return status instead of a pointer (It does everything for you)
uint32_t TAP_FillDataBE2(uint32_t Addr, uint32_t Len, const void *dataptr);
uint32_t TAP_FillDataBE4(uint32_t Addr, uint32_t Len, const void *dataptr);
uint32_t TAP_TargetStatus();

// Ugly shortcut to release target after a session
uint32_t runDamnit();

void *TAP_Execute            (uint64_t Ins, uint32_t iSz);
void *TAP_Execute_wSentData  (uint64_t Ins, uint32_t iSz, uint64_t Dat, uint32_t dSz);

void *TAP_ExecutePC          (uint64_t Ins, uint32_t iSz,               uint64_t PC, uint32_t pSz);
void *TAP_ExecutePC_wRecData (uint64_t Ins, uint32_t iSz, uint16_t dSz, uint64_t PC, uint32_t pSz);

#ifdef __cplusplus 
}
#endif
#endif
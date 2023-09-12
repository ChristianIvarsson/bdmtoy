#ifndef __CORE_WORKER_H
#define __CORE_WORKER_H
#ifdef __cplusplus 
extern "C" {
#endif

/// Queue-related functions
uint32_t  wrk_newQueue           (void *payloadptr); // Start a new queue of commands
uint32_t  wrk_queueReq           (void *payloadptr); // Add more commands to that queue
uint32_t  wrk_sendQueue          (                );
uint32_t  wrk_sendOneshot        (void *payloadptr); // Send a single command
uint32_t  wrk_sendOneshot_NoWait (void *payloadptr);

/// Reset flagged fault code. _MUST_ be called if you intend to retry
void      wrk_ResetFault         (                );

/// Polling 
uint32_t  busyOK                 (                ); // Report status, RET_OK means that it might be busy and there are no recorded faults
uint32_t  wrk_dumpDone           (                ); // Check if dump has completed. A value other than 0 means that it's done

/// Hackjobs...
void      wrk_PatchAddress       (uint32_t start  , uint32_t end      );  // Certain devices must be dumped in steps
void      wrk_byteSwapBuffer     (uint32_t noBytes, uint8_t  blockSize);  // Not really a hackjob.. byteswap buffer after a dump

// uint16_t *wrk_returnData        (uint16_t fromCmd                    );
uint16_t *wrk_requestData(void *payloadptr);
void      wrk_doubleImage(uint32_t End);
uint32_t  wrk_faultShortcut(uint32_t fault);
void      wrk_castProgPub(float percentage);
void      wrk_modifyEndAddress(uint32_t End);
uint32_t  wrk_pollFlashDone();
uint32_t  wrk_openFile(const char *fname);
uint32_t  wrk_writeFile(const char *fname, const uint32_t noBytes);
void      wrk_ConfigureTimeout(uint32_t newVal);
void     *hack_ReturnBufferPtr();
void      wrk_ResetflashDone();
void      wrk_ManualProgress();
#ifdef __cplusplus 
}
#endif
#endif
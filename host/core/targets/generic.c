#ifdef __cplusplus 
extern "C" {
#endif

#include "targets.h"
#include "../core_worker.h"

uint32_t flashGeneric()
{
    return RET_NOTINS;
}


uint32_t dumpGenericLE(uint32_t Start, uint32_t Length)
{
    // Send dump request
    uint32_t status = wrk_sendOneshot_NoWait( TAP_Dump(Start, Length) );

    if (status != RET_OK)
        return status;

    // Roll our thumbs
    while (busyOK() == RET_OK)
    {
        if (wrk_dumpDone() != RET_BUSY)
        {
            runDamnit(); // NTS; Don't do this! Just broke several targets without noticing...
            return busyOK();
        }
    }

    return RET_TIMEOUT;
}

uint32_t dumpGenericBE2(uint32_t Start, uint32_t Length)
{
    uint32_t retval = dumpGenericLE(Start, Length);
    
    if (retval == RET_OK)
        wrk_byteSwapBuffer(Length, 2);

    return retval;
}

uint32_t dumpGenericBE4(uint32_t Start, uint32_t Length)
{
    uint32_t retval = dumpGenericLE(Start, Length);
    
    if (retval == RET_OK)
        wrk_byteSwapBuffer(Length, 4);

    return retval;
}

#ifdef __cplusplus 
}
#endif

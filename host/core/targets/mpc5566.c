#ifdef __cplusplus 
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "../core.h"
#include "../core_worker.h"
#include "../core_requests.h"
#include "../core_requests_JTAG.h"
#include "../../../shared/insdef_ppc.h"

typedef struct  {
	uint8_t  MASKNUM_MIN: 4;
	uint8_t  MASKNUM_MAJ: 4;
	uint8_t  pad        : 4;
	uint8_t  PKG        : 3;
	uint8_t  CSP        : 1;
	uint16_t PARTNUM    :16;
} SIU_MIDR_BEONLE_t;

typedef struct {
    uint32_t WBBRLower;
    uint32_t WBBRUpper;
    uint32_t       MSR;
    uint32_t        PC;
    uint32_t        IR;
    uint32_t       CTL;
} OnCE_CPUSCR_t;

static uint32_t NEXUS_Step(uint64_t Ins, uint32_t Addr)
{
    OnCE_CPUSCR_t *CPUSCR;
    uint16_t *ptr;

    ptr = wrk_requestData( TAP_ExecutePC_wRecData(Ins, 4, 4, Addr, 4) );
    if (!ptr) {
        core_castText("Could not read data: \"%s\" ",core_TranslateFault());
        return 0xFFFF;
    }

    ptr = wrk_requestData( TAP_ReadRegister(NEXUS_CPUSCR, 24) );
    if (!ptr) {
        core_castText("Could not read data: \"%s\" ",core_TranslateFault());
        return 0xFFFF;
    }

    CPUSCR = (OnCE_CPUSCR_t *) &ptr[2];
/*
    core_castText("IR   : %08X", CPUSCR->IR);
    core_castText("PC   : %08X", CPUSCR->PC);*/
    core_castText("WBBRL: %08X", CPUSCR->WBBRLower);
    if (CPUSCR->WBBRUpper)
        core_castText("WBBRH: %08X", CPUSCR->WBBRUpper);
    // core_castText("CTL  : %08X", CPUSCR->CTL);
    // core_castText("MSR  : %08X", CPUSCR->MSR);
	return RET_OK;
}

static uint32_t set_FLASH_MCR(uint32_t value)
{
	uint32_t  data = 0;
	uint16_t *ptr;
    
    ptr = wrk_requestData( TAP_ReadDword(0xC3F88000) );
    if (!ptr) {
        core_castText("Could not read FLASH_MCR");
        return 0xFFFF;
    }

    data = ByteSwap(*(uint32_t *) &ptr[2]);
    core_castText("FLASH_MCR: %08X", data);
    data |= value;
    core_castText("toWrite: %08X", data);
    return wrk_sendOneshot( TAP_WriteDword(0xC3F88000, ByteSwap(data)) );
}

static uint32_t clr_FLASH_MCR(uint32_t value)
{
	uint32_t  data;
	uint16_t *ptr;
    
    ptr = wrk_requestData( TAP_ReadDword(0xC3F88000) );
    if (!ptr) {
        core_castText("Could not read FLASH_MCR");
        return 0xFFFF;
    }

    data = ByteSwap(*(uint32_t *) &ptr[2]);
    core_castText("FLASH_MCR: %08X", data);
    data &= ~value;
    core_castText("toWrite: %08X", data);
    return wrk_sendOneshot( TAP_WriteDword(0xC3F88000, ByteSwap(data)) );
}


static uint32_t NukeFlash()
{
	uint32_t  data, retval, i;
	uint16_t *ptr;
    uint16_t  readAddress[2];

    /* * * * * * * * * * * * */
    /* Negate primary locks */
// LMLR
    // Unlock LMLR
    retval = wrk_sendOneshot( TAP_WriteDword(0xC3F88004, ByteSwap(0xA1A11111)) );
    if (retval != RET_OK) {
        core_castText("Could not write");
        return retval;
    }

    // Negate locks (except shadow)
    retval = wrk_sendOneshot( TAP_WriteDword(0xC3F88004, ByteSwap(0x80100000)) );
    if (retval != RET_OK) {
        core_castText("Could not write");
        return retval;
    }

// HLR
    // Unlock HLR
    retval = wrk_sendOneshot( TAP_WriteDword(0xC3F88008, ByteSwap(0xB2B22222)) );
    if (retval != RET_OK) {
        core_castText("Could not write");
        return retval;
    }

    // Negate locks
    retval = wrk_sendOneshot( TAP_WriteDword(0xC3F88008, ByteSwap(0x80000000)) );
    if (retval != RET_OK) {
        core_castText("Could not write");
        return retval;
    }

    /* * * * * * * * * * * * * */
    /* Negate secondary locks */
// FLASH_SLMLR
    // Unlock FLASH_SLMLR
    retval = wrk_sendOneshot( TAP_WriteDword(0xC3F8800C, ByteSwap(0xC3C33333)) );
    if (retval != RET_OK) {
        core_castText("Could not write");
        return retval;
    }

    // Negate locks (except shadow)
    retval = wrk_sendOneshot( TAP_WriteDword(0xC3F8800C, ByteSwap(0x80100000)) );
    if (retval != RET_OK) {
        core_castText("Could not write");
        return retval;
    }


    /* * * * * * * * * * * */
    /* Enable high voltage */

    set_FLASH_MCR(4); // Set ERS

    /* * * * * * * * * * */
    /* Select partitions */
// FLASH_LMSR
    retval = wrk_sendOneshot( TAP_WriteDword(0xC3F88010, ~0) );
    if (retval != RET_OK) {
        core_castText("Could not write");
        return retval;
    }

// FLASH_HSR
    retval = wrk_sendOneshot( TAP_WriteDword(0xC3F88014, ~0) );
    if (retval != RET_OK) {
        core_castText("Could not write");
        return retval;
    }
    ptr = wrk_requestData( TAP_ReadDword(0xC3F88014) );
    if (!ptr) {
        core_castText("Could not read FLASH_MCR");
        return 0xFFFF;
    }
    data = ByteSwap(*(uint32_t *) &ptr[2]);
    core_castText("FLASH_HSR: %08X", data); 

    // Erase interlock write
    retval = wrk_sendOneshot( TAP_WriteDword(0x20000000, ~0) );
    if (retval != RET_OK) {
        core_castText("Could not write");
        return retval;
    }

    // EHV
    set_FLASH_MCR(1);
    
    // 0x400
    
    do
    {
        
        ptr = wrk_requestData( TAP_ReadDword(0xC3F88000) );
        if (!ptr) {
            core_castText("Could not read FLASH_MCR");
            return 0xFFFF;
        }
        data = ByteSwap(*(uint32_t *) &ptr[2]);
        core_castText("FLASH_MCR: %08X", data); 
        
        waitms(500);
    } while (!(data&0x400));
    
    
    core_castText("Flash erased");
    
    
    
    
    /*
    ptr = wrk_requestData( TAP_ReadDword(0xC3F90004) );
    if (!ptr) {
        core_castText("Could not read data");
        return 0xFFFF;
    }
    
    retval = wrk_sendOneshot( TAP_WriteDword(0xC3FC8000 + i, data) );
    if (retval != RET_OK) {
        core_castText("Could not write");
        return retval;
    }*/
    return RET_OK;
}


static uint32_t MPC5566_play()
{
	TAP_Config_host_t  config;
	SIU_MIDR_BEONLE_t  SIU_MIDR;
	uint32_t  data, retval, i;
	uint16_t *ptr;
    uint16_t  readAddress[2];

    config.Speed          = TAP_SPEED_CUSTOM;
    config.Custom         = 24000000;
    config.cfgmask.Endian = TAP_BIGENDIAN;

    wrk_ResetFault();

    ////////////////////////////////////////////
    // JTAG test
    core_castText("\n(In JTAGC mode)");

    retval = wrk_sendOneshot( JTAG_WriteIREG_w(5, 1) );
    if (retval != RET_OK) return retval;

    ptr = wrk_requestData( JTAG_ReadDREG(32) );
    if (!ptr) return 0xFFFF;

    core_castText("IDREG: %08X", *(uint32_t *) &ptr[2]);


    ////////////////////////////////////////////
    // NEXUS 1 / OnCE test
    core_castText("\nSwitching to NEXUS 1 / OnCE");

    config.Type = TAP_IO_NEXUS1;

    wrk_newQueue( TAP_SetInterface(config) );
    wrk_queueReq( TAP_TargetReady() );
    retval = wrk_sendQueue();
    if (retval != RET_OK) return retval;

    ptr = wrk_requestData( TAP_ReadRegDword(NEXUS_JTAGID) );
    if (!ptr) return 0xFFFF;

    core_castText("IDREG: %08X", *(uint32_t *) &ptr[2]);


    ////////////////////////////////////////////
    // NEXUS 3 test
    core_castText("\nSwitching to NEXUS 3");

    config.Type = TAP_IO_NEXUS3;

    wrk_newQueue( TAP_SetInterface(config) );
    wrk_queueReq( TAP_TargetReady() );
    retval = wrk_sendQueue();
    if (retval != RET_OK) return retval;

    // 0xC3F90004
    ptr = wrk_requestData( TAP_ReadDword(0xC3F90004) );
    if (!ptr) {
        core_castText("Could not read data");
        return 0xFFFF;
    }

    *(uint32_t *) &SIU_MIDR = ByteSwap(*(uint32_t *) &ptr[2]);
    core_castText("\nSIU_MIDR: %08X", *(uint32_t *) &SIU_MIDR);
    core_castText("PARTNUM: %04x" , SIU_MIDR.PARTNUM);
    core_castText("CSP    :    %u", SIU_MIDR.CSP);
    core_castText("PKG    :    %x", SIU_MIDR.PKG);
    core_castText("MASKMAJ:    %x", SIU_MIDR.MASKNUM_MAJ);
	core_castText("MASKMIN:    %x", SIU_MIDR.MASKNUM_MIN);

    return 0;
    
    ptr = wrk_requestData( TAP_ReadDword(0x20000000 + 0x4000) );
    if (!ptr) {
        core_castText("Could not read data");
        return 0xFFFF;
    }
    core_castText("FLASH: %08X", ByteSwap(*(uint32_t*)&ptr[2]));

    ptr = wrk_requestData( TAP_ReadDword(0x4000) );
    if (!ptr) {
        core_castText("Could not read data");
        return 0xFFFF;
    }
    core_castText("FLASH: %08X", ByteSwap(*(uint32_t*)&ptr[2]));


    ptr = wrk_requestData( TAP_ReadDword(0xC3F84000) );
    if (!ptr) {
        core_castText("Could not read data");
        return 0xFFFF;
    }
    core_castText("EBI_MCR: %08X", ByteSwap(*(uint32_t*)&ptr[2]));




    // retval = wrk_sendOneshot( JTAG_WriteIREG_w(5, 1) );
    // if (retval != RET_OK) return retval;

    // FLASH_MCR
    ptr = wrk_requestData( TAP_ReadDword(0xC3F88000) );
    if (!ptr) {
        core_castText("Could not read data");
        return 0xFFFF;
    }
    core_castText("FLASH_MCR: %08X", ByteSwap(*(uint32_t*)&ptr[2]));

    // 0x0b603600

    // FLASH_BIUAPR
    ptr = wrk_requestData( TAP_ReadDword(0xC3F88020) );
    if (!ptr) {
        core_castText("Could not read data");
        return 0xFFFF;
    }
    core_castText("FLASH_BIUAPR: %08X", ByteSwap(*(uint32_t*)&ptr[2]));

    retval = wrk_sendOneshot( TAP_WriteDword(0xC3F88020, ~0) );
    if (retval != RET_OK) {
        core_castText("Could not write");
        return retval;
    }
    
    

    
    // return RET_OK;
/*
    ptr = wrk_requestData( TAP_ReadDword(0xC3F88020) );
    if (!ptr) {
        core_castText("Could not read data");
        return 0xFFFF;
    }
    core_castText("FLASH_BIUAPR: %08X", ByteSwap(*(uint32_t*)&ptr[2]));
    ptr = wrk_requestData( TAP_ReadDword(0xC3F88020) );
    if (!ptr) {
        core_castText("Could not read data");
        return 0xFFFF;
    }
    core_castText("FLASH_BIUAPR: %08X", ByteSwap(*(uint32_t*)&ptr[2]));

*/


    // 0xFFFF_F000
    ptr = wrk_requestData( TAP_ReadDword(0xFFFFF000) );
    if (!ptr) {
        core_castText("Could not read data");
        return 0xFFFF;
    }
    core_castText("BAM: %08X", ByteSwap(*(uint32_t*)&ptr[2]));


    ptr = wrk_requestData( TAP_ReadDword(0xC3F8801C) );
    if (!ptr) {
        core_castText("Could not read data");
        return 0xFFFF;
    }
    core_castText("FLASH_BIUCR: %08X", ByteSwap(*(uint32_t*)&ptr[2]));

    // 0x00FFFDE0
    /*
    ptr = wrk_requestData( TAP_ReadDword(0x00FFFDE0) );
    if (!ptr) {
        core_castText("Could not read flash[0]");
        return 0xFFFF;
    }
    core_castText("flash[0]: %08X", ByteSwap(*(uint32_t*)&ptr[2]));*/

/*
    core_castText("Reading TPURAM");
    ptr = wrk_requestData( TAP_ReadDword(0xC3FC8000) );
    if (!ptr) {
        core_castText("Could not read data: \"%s\" ",core_TranslateFault());
        return 0xFFFF;
    }
    core_castText("Data: %08X", *(uint32_t*) &ptr[2]);
*/
    
    /*
    ptr = wrk_requestData( TAP_ExecutePC_wRecData(data, 4, 8, 0xC3FC8000, 4    ) );
    if (!ptr) {
        core_castText("Could not read data: \"%s\" ",core_TranslateFault());
        return 0xFFFF;
    }*/


    ////////////////////////////////////////////////////
    ///////////////////////////////////////////////////
    // Toys
/*
    data = PPC_be_MFMSR_R0;
    core_castText("Mem ins: %08X", data);
    
    core_castText("Writing TPURAM");
    for (i = 0; i < 64; i+=4)
    {
        retval = wrk_sendOneshot( TAP_WriteDword(0xC3FC8000 + i, data) );
        if (retval != RET_OK) {
            core_castText("Could not write");
            return retval;
        }
    }
    retval = wrk_sendOneshot( TAP_WriteRegister(NEXUS_CPUSCR_MSR, 4, ByteSwap(0x00000000)) );
    if (retval != RET_OK) {
        core_castText("Could not write");
        return retval;
    }
    */
/*
    retval = NEXUS_Step( PPC_be_LI(1, 4), 0xC3FC8000);
    if (retval != RET_OK) {
        core_castText("Could not step");
        return retval;
    }
*/


    // retval = NEXUS_Step( PPC_be_MFSPR_R0(627), 0xC3FC8000);
    // if (retval != RET_OK) return retval;

    // 624 - 630: mas0 - mas6
    
    // 624: mas0
    // 625: mas1
    // 626: mas2
    // 627: mas3
    // 628: mas4
    // 629: mas5
    // 630: mas6
/*
    *(uint32_t*) &readAddress = 0x00FFFDE0;

    retval = NEXUS_Step( PPC_be_LIS(1, readAddress[1]), 0xC3FC8000);
    if (retval != RET_OK) return retval;

    retval = NEXUS_Step( PPC_be_ORI(1, 1, readAddress[0]), 0xC3FC8000);
    if (retval != RET_OK) return retval;

    retval = NEXUS_Step( PPC_be_LWZ(1, 1, 0), 0xC3FC8000);
    if (retval != RET_OK) return retval;
*/
    // return RET_OK; 
    return NukeFlash();
}


uint32_t initMPC5566()
{
    TAP_Config_host_t config;
    uint32_t retval;

    config.Type           = TAP_IO_JTAG;
    config.Speed          = TAP_SPEED_CUSTOM;
    config.Custom         = 24000000;
    config.cfgmask.Endian = TAP_BIGENDIAN;

    wrk_ResetFault();

    wrk_newQueue( TAP_SetInterface(config) );
    wrk_queueReq( TAP_TargetInitPort() ); // Workaround for a known problem: Force return tap to jtagc
    wrk_queueReq( TAP_TargetReset() );
    wrk_queueReq( TAP_TargetReady() );
	retval = wrk_sendQueue();
    if (retval != RET_OK) return retval;

    // return RET_OK;
	return MPC5566_play();
    // return wrk_sendQueue();
}


static uint32_t NEXUS_weirdStep(uint64_t Ins, uint32_t Addr)
{
    return wrk_sendOneshot( TAP_ExecutePC(Ins, 4, Addr, 4) );
}

static void NEXUS_PrintFLASHCR()
{
    uint16_t *ptr = wrk_requestData( TAP_ReadDword(0xC3F88000) );
    if (!ptr) {
        core_castText("Could not dump data");
        return 0xFFFF;
    }
    core_castText("FlashMCR: %08X", ((*(uint32_t*)&ptr[2])));
}

static void NEXUS_ReadRSR()
{
    uint16_t *ptr = wrk_requestData( TAP_ReadDword(0xC3F9000C) );
    if (!ptr) {
        core_castText("Could not dump data");
        return 0xFFFF;
    }
    core_castText("RSR: %08X", ByteSwap((*(uint32_t*)&ptr[2])));
}




uint32_t nexusDump(uint32_t Start, uint32_t Length)
{
    
    uint32_t *bufferPtr = hack_ReturnBufferPtr();
    uint16_t *ptr;
    uint32_t retval;
    uint32_t End = Length + Start;
    uint32_t i;
    uint32_t data;
    OnCE_CPUSCR_t *CPUSCR;
    
    uint32_t oldReg[32];
    uint32_t newReg[32];
    uint32_t *ptr32;
    uint32_t e;
    // Start = 0x4010;
    Start = 0xFFFFFFFC;
    CPUSCR = (OnCE_CPUSCR_t *) &oldReg[0];

    // wrk_sendOneshot( TAP_WriteDword(0xC3F90010, ByteSwap(0x80000000)) );
    waitms(50);
    // *(volatile uint32_t*)0xC3F90010 = 0x80000000;
    for (i = 0; i < 5; i++)
    {
        core_castText(" ");
        // NEXUS_PrintFLASHCR();
        core_castText("Stepping: %04x", Start);

        if (!(ptr = wrk_requestData( TAP_ReadDword(Start) )))
        {   core_castText("Could not dump data"); return 0xFFFF; }
        core_castText("Dat: %08X", ((*(uint32_t*)&ptr[2])));
/*
        // Execute instruction at address
        if (NEXUS_weirdStep(  ByteSwap((*(uint32_t*)&ptr[2])), Start))
        {   core_castText("Step failed!"); return 1; }
*/

        // Read current PC
        if (!(ptr = wrk_requestData( TAP_ReadRegister(NEXUS_CPUSCR, 24) )))
        {   core_castText("Could not read data: \"%s\" ",core_TranslateFault()); return 0xFFFF; }

        CPUSCR = (OnCE_CPUSCR_t *) &ptr[2];
        core_castText("WBBRL: %08X", CPUSCR->WBBRLower);
        core_castText("CTL  : %08X", CPUSCR->CTL);
        core_castText("PC   : %08X", CPUSCR->PC);
        core_castText("MSR   : %08X", CPUSCR->MSR);

        // Update address to read / run from
        // Start = CPUSCR->PC;
    }
    
    
    
    
    // return 1;
    /*
    do
    {
        ptr = wrk_requestData( TAP_ReadDword(Start) );
        if (!ptr) {
            core_castText("Could not dump data");
            return 0xFFFF;
        }

        wrk_castProgPub(( (float)Start/(float)End)*100.0);
        *bufferPtr++ = (*(uint32_t*)&ptr[2]);
        Start += 4;
        Length -= 4;
    } while (Length);
    
    core_castText("Done");
    */
    return RET_OK;
}


uint32_t dumpSRAM_mpc5566(uint32_t Start, uint32_t Length)
{
    uint32_t *bufferPtr;
    uint32_t currCount = 0, lastCount = Length;
    uint16_t *ptr;

    core_castText("Dumping SRAM %08X", Start);

    if (!(bufferPtr = hack_ReturnBufferPtr())) {
        core_castText("Got no pointer!");
        return 1;
    }

    do
    {
        if (!(ptr = wrk_requestData( TAP_ReadDword(Start) ))) {
            core_castText("Could not dump sram");
            return 0xFFFF;
        }

        *bufferPtr++ = *(uint32_t*)&ptr[2];
        Start  += 4;
        Length -= 4;
        currCount+=4;
        wrk_castProgPub(((float)currCount/lastCount)*100.0);
    } while (Length);
    return RET_OK;
}

#ifdef __cplusplus 
}
#endif

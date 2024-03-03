
#include "../bdmstuff.h"

#include "target_helper.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Generic helpers

bool target_helper::printReglist( const regSummary_t *regs, size_t count ) {

    static const char *noName = "???";
    uint32_t temp[ 8 ];
    const char *ptr;

    core.queue.reset();

    for ( uint32_t i = 0; i < count; i++ )
        core.queue += readRegister( regs[ i ].regCmd, regs[ i ].regSize );

    if ( core.queue.send() == false ) {
        core.castMessage("Error: Unable to request register data");
        return false;
    }

    core.castMessage("- -");
    core.castMessage("- - R E G I S T E R - S U M M A R Y");
    core.castMessage("- -");

    for ( uint32_t i = 0; i < count; i++ ) {
        if ( core.getData( (uint16_t*)temp, TAP_DO_READREGISTER, regs[ i ].regSize, i ) == false ) {
            core.castMessage("Error: Unable to retrieve register data");
            return false;
        }

        for ( uint32_t sp = 0; sp < regs[ i ].spacing; sp++ )
            core.castMessage("- -");

        ptr = ( regs[ i ].name == nullptr || regs[ i ].name[0] == 0 ) ? noName : regs[ i ].name;

        switch ( regs[ i ].regSize ) {
        case sizeByte:
            core.castMessage("- - %s   %16x", ptr, (uint8_t)temp[ 0 ]);
            break;
        case sizeWord:
            core.castMessage("- - %s   %16x", ptr, (uint16_t)temp[ 0 ]);
        break;
        case sizeDword:
            core.castMessage("- - %s   %16x", ptr, temp[ 0 ]);
            break;
        case sizeQword:
            core.castMessage("- - %s   %16x", ptr, (uint64_t*)temp);
            break;
        default:
            core.castMessage("- - %s   -- unkown size --");
            break;
        }
    }

    core.castMessage("- -");

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPU32

#include "../requests_cpu32.h"

static const regSummary_t CPU32_regs[] = {
    // System registers
    { 0x2580 + CPU32_SREG_STS , sizeDword, 0, "SR   " }, // Status Register
    { 0x2580 + CPU32_SREG_PCC , sizeDword, 1, "PCC  " }, // Current Instruction Program Counter
    { 0x2580 + CPU32_SREG_PC  , sizeDword, 0, "RPC  " }, // Return Program Counter
    { 0x2580 + CPU32_SREG_USP , sizeDword, 0, "USP  " }, // User Stack Pointer
    { 0x2580 + CPU32_SREG_SSP , sizeDword, 0, "SSP  " }, // Supervisor Stack Pointer
    { 0x2580 + CPU32_SREG_VBR , sizeDword, 1, "VBR  " }, // Vector Base Register
    { 0x2580 + CPU32_SREG_TMPA, sizeDword, 0, "ATEMP" }, // Temporary Register A
    { 0x2580 + CPU32_SREG_FAR , sizeDword, 0, "FAR  " }, // Fault Address Register
    { 0x2580 + CPU32_SREG_SFC , sizeDword, 0, "SFC  " }, // Source Function Code Register
    { 0x2580 + CPU32_SREG_DFC , sizeDword, 0, "DFC  " }, // Destination Function Code Register
    // Regular registers
    { 0x2180 + 0              , sizeDword, 1, "D0   " },
    { 0x2180 + 1              , sizeDword, 0, "D1   " },
    { 0x2180 + 2              , sizeDword, 0, "D2   " },
    { 0x2180 + 3              , sizeDword, 0, "D3   " },
    { 0x2180 + 4              , sizeDword, 0, "D4   " },
    { 0x2180 + 5              , sizeDword, 0, "D5   " },
    { 0x2180 + 6              , sizeDword, 0, "D6   " },
    { 0x2180 + 7              , sizeDword, 0, "D7   " },
    { 0x2180 + 8              , sizeDword, 1, "A0   " },
    { 0x2180 + 9              , sizeDword, 0, "A1   " },
    { 0x2180 + 10             , sizeDword, 0, "A2   " },
    { 0x2180 + 11             , sizeDword, 0, "A3   " },
    { 0x2180 + 12             , sizeDword, 0, "A4   " },
    { 0x2180 + 13             , sizeDword, 0, "A5   " },
    { 0x2180 + 14             , sizeDword, 0, "A6   " },
    { 0x2180 + 15             , sizeDword, 0, "A7   " },
};

bool helper_CPU32::printRegisters() {
    return printReglist( CPU32_regs, sizeof(CPU32_regs) / sizeof(CPU32_regs[0]) );
}

bool helper_CPU32::runPC( uint64_t pc ) {
    bool retVal;
    uint16_t status;

    if ( !core.queue.send(writeSystemRegister( 0, pc )) ) {
        core.castMessage("Error: runPC - Could not set program counter");
        return false;
    }

    // Run from
    if ( !core.queue.send(targetStart()) ) {
        core.castMessage("Error: runPC - Could not start target");
        return false;
    }

    // Wait for target stop
    do {
        retVal = core.getStatus( &status );
        // Let's be nice. No need to absolutely hammer the status request
        sleep_ms( 50 );
    } while ( retVal && status == RET_TARGETRUNNING );

    if ( status != RET_TARGETSTOPPED ) {
        core.castMessage("Error: runPC - Could not stop target");
        return false;
    }

    return true;
}

bool helper_CPU32::setPC( uint64_t pc ) {
    if ( !core.queue.send(writeSystemRegister( 0, pc )) ) {
        core.castMessage("Error: setPC - Could not set program counter");
        return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Other..

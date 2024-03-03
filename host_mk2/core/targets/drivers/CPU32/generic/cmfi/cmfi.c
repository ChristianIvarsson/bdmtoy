#include "cmfi.h"

extern regData_t   regData;
extern eraseData_t eraseData;
extern writeData_t writeData;

/*
While doing this, I found a bug in legion t8. 
MCP doesn't perform margin reads as it should
- Uppon further reading, you don't have to perform a full read on cmfi v5.0 and 5.1
- Left for now since I'm not sure which source to trust.
*/

// Watchdog must be serviced every once in a while or the system will reset
void serviceWatchdog( void )
{
    *regData.SWSR = 0x55;
    *regData.SWSR = 0xAA;
}

uint32_t write( uint16_t  nWords,
                uint32_t  destination,
                int32_t  *source )
{
    // Reflects status of the last operation before it gave up or completed what it was doing
    uint32_t status = STATUS_OK;

    // uint32_t hardTries = 10;

    // Convert to nDwords
    uint32_t nDwords = nWords / 2;

    serviceWatchdog();

    if ( writeData.maxProgramPulses == 0 )
        return STATUS_NOPULSE;

    if ( (destination & 63) != 0 ||
         (nDwords     & 15) != 0 )
        return STATUS_ALIGN;

    if ( nDwords == 0 )
        return STATUS_NODATA;

    // Note about pawsProgPulses[]
    // FFFF = Keep pulsing until data match or maxProgramPulses has been reached
    // Otherwise pulse for x steps, update PAWS / index etc

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Motorola's sequence: ( Can't do it exactly like this since we write pages sequentially, they do it in parallel )

    //  1. Set SIE if in shadow                                          ( *CMFIMCR |= 0x2000                      )
    //  2. copy N prog pulses from pawsProgPulses[ 0 ]
    //  3. Negate PROTECT                                                ( *CMFIMCR &= 0xBFFF                      )
    //  4. Clear NVR, PAWS, GDB                                          ( *CMFITST &= 0xF0DF                      )
    //  5. OR pawsProgData[ 0 ]                                          ( *CMFITST |= pawsProgData[ 0 ]           )
    //  6. Generate a partition bitmask in the stupidest way imaginable
    //  7. Clear SCLKR, CLKPE, CLKPM in CTL1. Clear BLOCK, PE in CTL2    ( *CMFICTL &= 0xC48000FB                  )
    //  8. OR ctlProg[ 0 ] and sector mask into CTL1 and CTL2            ( *CMFICTL |= ( ctlProg[ 0 ] | mask )     )
    //  9. OR in partition bitmask                                       ( . . .                                   )
    // 10. Set SES in CTL2                                               ( *CMFICTL |= 2                           )

    // 11. Write page ( all 64 bytes ), store where the first non-ffff_ffff was written

// pulseMore: //

    // xx. if mask == 0, status = OK and goto // wrapUp //

    // 12. Check if / update PAWS
    // < if PAWS update >
    //     pulseIdx++
    //     Clear NVR, PAWS, GDB                                          ( *CMFITST &= 0xF0DF                      )
    //     OR pawsProgData[ pulseIdx ]                                   ( *CMFITST |= pawsProgData[ pulseIdx ]    )
    //     Clear SCLKR, CLKPE, CLKPM in CTL1                             ( *CMFICTL &= 0xC480FFFF                  )
    //     OR ctlProg[ pulseIdx ]                                        ( *CMFICTL |= ctlProg[ pulseIdx ]         )

    // 13. Set EHV in CTL2                                               ( *CMFICTL |= 1                           )
    // 14. Wait for HVS to become 0
    // 15. Negate EHV

    // 16. Perform one of the margin read methods
    //     1. Perform cheap margin read as:
    //        Read page address 0 and 32. Don't do anything with this data
    //        Read offset stored during step 11
    //        if margin at offset from step 11 passed, disable method 1
    //        - - goto // chkPulseCnt //
    //     2. Perform full margin read
    //        Read page offset 0 + verifiedTo and 32 + verifiedTo. If both passed, increment verifiedTo and check again
    //        Do this until the whole page has been verified or a non-match has been found
    //        If passed, remove from mask ( d4 )
    //        - - goto // chkPulseCnt //

// chkPulseCnt: //
    // 17. Increment pulseCnt
    //     if maxProgramPulses > pulseCnt, goto // pulseMore //

// wrapUp: //
    // 18. Negate SES                                                    ( *CMFICTL &= 0xFFFFFFFD                  )
    // 19. Check status of previous operations. if fail, leave

    // Negate PROTECT bit
    *regData.CMFIMCR &= ~MCR_PROTECT;

    while ( nDwords != 0 )
    {
        // AND to wrap around shadow data to where it belongs
        volatile int32_t *dst = (int32_t*)(regData.base + (destination & 0x3ffff));
        int32_t *cpy = (int32_t*)dst;
        int32_t *sr = source;

        // Regular partitions are 32 K, shadow 256 B.
        // Since shadow is where it is, this is going to wrap around due to AND 7. - That is, destination >= 4_0000 becomes bit / partition 0 again.
        // This is OR'd as is with CTL2 where bits 15:8 contains the partition bitmask. We're actually OR'ing with CTL1 nd CTL2 but BE be doing BE stuff...
        uint32_t mask = 0x0100 << ((destination >> 15) & 7);

        int32_t  firstOffset = -1;
        uint16_t pulseCnt = 0;
        uint32_t pulseIdx = 0;
        uint32_t preventPaws = 0;
        uint16_t nextPulseMatch = writeData.pawsProgPulses[ 0 ];
        uint32_t cheapMargin = 1;
        uint32_t verifiedTo = 0;

        serviceWatchdog();

        if ( destination >= 0x40000 )
        {
            // Shadow doesn't have more than 256 bytes
            if ( nDwords > 64 )
                nDwords = 64;

            *regData.CMFIMCR |= MCR_SIE;
        }
        else
        {
            *regData.CMFIMCR &= ~MCR_SIE;
        }

        // Clear NVR, PAWS, GDB and OR in new data
        *regData.CMFITST = ( *regData.CMFITST & 0x0000F0DF ) | writeData.pawsProgData[ 0 ];

        // CMFICTL1 - Negate SCLKR, CLKPE, CLKPM
        // CMFICTL2 - Negate BLOCK, PE
        // OR in ctlProg[ 0 ] and partition mask
        *regData.CMFICTL = ( *regData.CMFICTL & 0xC48000FB ) | writeData.ctlProg[ 0 ] | mask;

        // Going live
        *regData.CMFICTL |= CTL_SES;

        // Fill page buffer
        for ( int16_t i = 0; i < 16; i++ )
        {
            if ( *sr != -1 && firstOffset == -1 )
                firstOffset = i;

            *cpy++ = *sr++;
        }

        if ( firstOffset != - 1 )
        {
            while ( 1 )
            {
                if ( preventPaws == 0 &&
                     pulseCnt    == nextPulseMatch )
                {
                    pulseIdx++;

                    if ( writeData.pawsProgPulses[ pulseIdx ] == 0xffff )
                        preventPaws = 1;
                    else
                        nextPulseMatch += writeData.pawsProgPulses[ pulseIdx ];

                    // Clear NVR, PAWS and GDB and OR in new data
                    *regData.CMFITST = ( *regData.CMFITST & 0x0000F0DF ) | writeData.pawsProgData[ pulseIdx ];

                    // Clear SCLKR, CLKPE and CLKPM in CTL1
                    *regData.CMFICTL = ( *regData.CMFICTL & 0xC480FFFF ) | writeData.ctlProg[ pulseIdx ];
                }

                *regData.CMFICTL |= CTL_EHV;

                do {
                    serviceWatchdog();
                } while ( *regData.CMFICTL & CTL_HVS );

                *regData.CMFICTL &= ~CTL_EHV;

                if ( cheapMargin )
                {
                    // Don't know why they did this.
                    // RM makes no mention of doing some cheap margin once and force an additional pulse
                    asm volatile (
                        "move.l   0(%0), %%d0\n"
                        "or.l    32(%0), %%d0\n"
                        :: "r" (dst) : "d0", "cc"
                    );

                    if ( dst[ firstOffset ] == 0 )
                        cheapMargin = 0;
                }
                else
                {
                    // Perform margin reads on both halves, OR them and check if all bits are zero
                    if ( (dst[ verifiedTo + 8 ] | dst[ verifiedTo ]) == 0 )
                    {
                        if ( ++verifiedTo >= 8 )
                            break;
                    }
                }

                // Check count
                if ( ++pulseCnt >= writeData.maxProgramPulses )
                {
                    status = STATUS_MAXPULSE;
                    *regData.CMFICTL &= ~CTL_SES;
                    goto writeDone;
                }
            }
            
        }

        *regData.CMFICTL &= ~CTL_SES;

        // This makes the whole thing crash and I don't understand why...
/*
        // Perform a hard check on written data
        cpy = (uint32_t*)dst;
        sr = source;

        uint32_t hardMatched = 1;

        for ( uint32_t i = 0; i < 16; i++ )
        {
            // Reset the whole loop if data is not matched
            // Not optimal since counts etc will reset but hey
            if ( *sr++ != *cpy++ )
            {
                if ( --hardTries == 0 )
                {
                    status = STATUS_MAXPULSE;
                    goto writeDone;
                }

                hardMatched = 0;
                break;
            }
        }
*/
        // if ( hardMatched )
        {
            destination += 64;
            source += 16;
            nDwords -= 16;
        }
    }

writeDone:

    *regData.CMFIMCR &= ~MCR_SIE;
    *regData.CMFICTL &= ~CTL_BLOCK;

    *regData.CMFIMCR |= MCR_PROTECT;

    return status;
}

static uint32_t ffConfirm( uint32_t address, uint32_t nDwords )
{
    int32_t *src = (int32_t*)address;

    while ( nDwords-- )
    {
        if ( *src++ != -1 )
            return 0;
    }

    return 1;
}

static uint32_t ffMask( uint32_t mask )
{
    // Scan for FF
    for ( uint32_t m = 0; m < 8; m++ )
    {
        // Each sector is 32k
        uint32_t address = regData.base + (m << 15);

        // Sector set
        if ( (mask & (1 << m)) != 0 )
        {
            // Sector 0 contains shadow
            if ( m == 0 )
            {
                *regData.CMFIMCR |= MCR_SIE;
                if ( !ffConfirm( address, 256 / 4 ) )
                {
                    *regData.CMFIMCR &= ~MCR_SIE;
                    continue;
                }
            }

            *regData.CMFIMCR &= ~MCR_SIE;

            if ( ffConfirm( address, 32768 / 4 ) )
            {
                mask &= ~(1 << m);
            }
        }
    }

    return mask & 0xff;
}

uint32_t sectorErase( uint32_t mask )
{
    uint16_t pulseCnt = 0;
    uint16_t pulseIdx = 0;
    uint32_t status = STATUS_OK;
    uint16_t preventPaws = 0;
    uint16_t nextPulseMatch = eraseData.pawsErasePulses[ 0 ];
    uint8_t  pulseMod = eraseData.pawsEraseMode[ 0 ];

    serviceWatchdog();

    if ( (mask = ffMask( mask )) == 0 )
        return status;

    *regData.CMFIMCR &= ~MCR_PROTECT;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Motorola's sequence

    //  0. Copy pawsEraseMode[ 0 ] to modeBack                           ( Yes, I spotted it much later. Cry me a river )
    //  1. Negate PROTECT                                                ( *CMFITST &= 0xBFFF                      )
    //  2. Clear NVR, PAWS, GDB                                          ( *CMFITST &= 0xF0DF                      )
    //  3. OR pawsEraseData[ 0 ]                                         ( *CMFITST |= pawsEraseData[ 0 ]          )
    //  4. Generate a partition bitmask in the stupidest way imaginable
    //  5. Clear SCLKR, CLKPE, CLKPM in CTL1. Clear BLOCK in CTL2        ( *CMFICTL &= 0xC48000FF                  )
    //  8. OR ctlErase[ 0 ], sector mask and PE into CTL1 and CTL2       ( *CMFICTL |= ( ctlErase[ 0 ] | mask | 4 ))
    //  9. Set SES                                                       ( *CMFICTL |= 2                           )
    // 10. Interlock write                                               ( (uint32_t*)base = 0xffffffff            )
    // 11. Check maxErasePulses. Abort if 0

// pulseMore: //

    // 12. Check if / update PAWS
    // < if PAWS update >
    //     pulseIdx++
    //     Clear NVR, PAWS, GDB                                          ( *CMFITST &= 0xF0DF                      )
    //     OR pawsEraseData[ pulseIdx ]                                  ( *CMFITST |= pawsEraseData[ pulseIdx ]   )
    //     Clear SCLKR, CLKPE, CLKPM in CTL1                             ( *CMFICTL &= 0xC480FFFF                  )
    //     OR ctlErase[ pulseIdx ]    
    //     Copy pawsEraseMode[ pulseIdx ] to modeBack

    // 13. Set EHV in CTL2                                               ( *CMFICTL |= 1                           )
    // 14. Wait for HVS to become 0
    // 15. Negate EHV

    // 16. Read modeBack. if 0x80, jump to // chkPulseCnt // 

    // 17. Check partitions. Erase from mask if margin passed

// chkPulseCnt: //
    // 18. Increment pulseCnt
    //     if maxErasePulses > pulseCnt, goto // pulseMore //

    // Clear NVR, PAWS, GDB and OR in new data
    *regData.CMFITST = ( *regData.CMFITST & 0x0000F0DF ) | eraseData.pawsEraseData[ 0 ];

    // CMFICTL1 - Negate SCLKR, CLKPE, CLKPM
    // CMFICTL2 - Negate BLOCK
    // OR in ctlErase[ 0 ] and partition mask
    // Note -They never change the stored partition mask, even after verification
    // - It is not known if they have a reason for this or not. All I know is that I'm f***ing tired of CMFI so let's do it EXACTLY as they do it...
    *regData.CMFICTL = ( *regData.CMFICTL & 0xC48000FF ) | eraseData.ctlErase[ 0 ] | ((mask << 8) & 0xff00) | CTL_PE;

    // Going live
    *regData.CMFICTL |= CTL_SES;

    // Erase interlock
    *(volatile int32_t*)regData.base = -1;

    while ( mask )
    {
        if ( pulseCnt >= eraseData.maxErasePulses )
        {
            status = STATUS_MAXPULSE;
            goto eraseDone;
        }

        if ( preventPaws == 0 &&
             pulseCnt    == nextPulseMatch )
        {
            pulseIdx++;

            if ( eraseData.pawsErasePulses[ pulseIdx ] == 0xffff )
                preventPaws = 1;
            else
                nextPulseMatch += eraseData.pawsErasePulses[ pulseIdx ];

            // Clear NVR, PAWS and GDB and OR in new data
            *regData.CMFITST = ( *regData.CMFITST & 0x0000F0DF ) | eraseData.pawsEraseData[ pulseIdx ];

            // Clear SCLKR, CLKPE and CLKPM in CTL1
            *regData.CMFICTL = ( *regData.CMFICTL & 0xC480FFFF ) | eraseData.ctlErase[ pulseIdx ];

            pulseMod = eraseData.pawsEraseMode[ pulseIdx ];
        }

        *regData.CMFICTL |= CTL_EHV;

        do {
            serviceWatchdog();
        } while ( *regData.CMFICTL & CTL_HVS );

        *regData.CMFICTL &= ~CTL_EHV;

        if ( pulseMod != 0x80 )
            mask = ffMask( mask );

        pulseCnt++;
    }

eraseDone:

    *regData.CMFITST &= ~( TST_NVR | TST_PAWS | TST_GDB );

    *regData.CMFICTL &= ~CTL_SES;

    *regData.CMFICTL &= ~CTL_BLOCK;

    *regData.CMFIMCR |= MCR_PROTECT;

    return status;
}

uint32_t bulkErase( void )
{
    return sectorErase( 0xff );
}

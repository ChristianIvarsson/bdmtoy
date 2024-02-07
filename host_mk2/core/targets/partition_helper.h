#ifndef __PARTITION_HELPER_H__
#define __PARTITION_HELPER_H__

#include "flash/flash.h"

enum eFlashWidth : uint32_t {
    enWidth8     = (1 << 0),
    enWidth16    = (1 << 1),
    enWidth32    = (1 << 2)
};

class parthelper {

    static const dids_t *ofWidth( const didcollection_t & dids, const eFlashWidth & width ) {
        switch ( width ) {
        case enWidth8:  return &dids.x8parts;
        case enWidth16: return &dids.x16parts;
        case enWidth32: return &dids.x32parts;
        default:        return nullptr;
        }
    }

public:
    parthelper() {}

/*
#define MID_WINBOND    ( 0x00DA )
*/

    static const flashpart_t *getMap( uint32_t mid, uint32_t did, eFlashWidth width ) {
        const dids_t *dids = nullptr;
        switch ( mid ) {
        case MID_AMD:      dids = ofWidth( amd_dids     , width ); break; /* 0001 */
        case MID_FUJITSU:  dids = ofWidth( fujitsu_dids , width ); break; /* 0004 */
        case MID_EON:      dids = ofWidth( eon_dids     , width ); break; /* 001C */
        case MID_ATMEL:    dids = ofWidth( atmel_dids   , width ); break; /* 001F */
        case MID_ST:       dids = ofWidth( st_dids      , width ); break; /* 0020 */
        case MID_CATALYST: dids = ofWidth( catalyst_dids, width ); break; /* 0031 */
        case MID_AMIC:     dids = ofWidth( amic_dids    , width ); break; /* 0037 */
        case MID_INTEL:    dids = ofWidth( intel_dids   , width ); break; /* 0089 */
        case MID_MXIC:     dids = ofWidth( mxic_dids    , width ); break; /* 00C2 */
        default:
            break;
        }

        if ( dids == nullptr || dids->count == 0 )
            return nullptr;

        for ( uint32_t i = 0; i < dids->count; i++ ) {
            if ( dids->parts[ i ].did == did )
                return &dids->parts[ i ];
        }

        return nullptr;
    }

    static const char *const getManufacturerName( uint32_t mid ) {
        switch ( mid ) {
        case MID_AMD:      return "AMD";                        /* 0001 */
        case MID_FUJITSU:  return "Fujitsu";                    /* 0004 */
        case MID_EON:      return "EON";                        /* 001C */
        case MID_ATMEL:    return "Atmel";                      /* 001F */
        case MID_ST:       return "ST";                         /* 0020 */
        case MID_CATALYST: return "Catalyst / CSI";             /* 0031 */
        case MID_AMIC:     return "AMIC";                       /* 0037 */
        case MID_INTEL:    return "Intel or Texas Instruments"; /* 0089 */
        case MID_MXIC:     return "MXIC";                       /* 00C2 */
        default: return "Unknown";
        }
    }
};

#endif

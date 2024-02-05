#ifndef __PARTITION_HELPER_H__
#define __PARTITION_HELPER_H__

#include "flash_partitions.h"

#define MID_AMD        ( 0x0001 )



enum eFlashWidth : uint32_t {
    enWidth8     = (1 << 0),
    enWidth16    = (1 << 1),
    enWidth32    = (1 << 2)
};

class parthelper {

    static const dids_t *ofWidth( const didcollection_t & dids, eFlashWidth width ) {
        switch ( width ) {
        case enWidth8:   return &dids.x8parts;
        case enWidth16:  return &dids.x16parts;
        case enWidth32:  return &dids.x32parts;
        default:         return nullptr;
        }
    }

public:
    parthelper() {}

    static const flashpart_t *getMap( uint32_t mid, uint32_t did, eFlashWidth width ) {
        const dids_t *dids = nullptr;
        switch ( mid ) {
        case MID_AMD:
            dids = ofWidth( amd_dids, width );
            break;
        default:
            break;
        }

        if ( dids == nullptr)
            return nullptr;

        if ( dids->count == 0 )
            return nullptr;

        for ( uint32_t i = 0; i < dids->count; i++ ) {
            if ( dids->parts[ i ].did == did )
                return &dids->parts[ i ];
        }

        return nullptr;
    }
};

#endif

#ifndef __GENERIC_H__
#define __GENERIC_H__

#include "../bdmstuff.h"
#include "targets.h"

class genericRead_BE4 : public virtual iTarget {
    // bdmstuff & genCore;
public:
    explicit genericRead_BE4( bdmstuff & mc )
        : iTarget( mc ) {}

    bool read( const target_t *, const memory_t *region) {
        // genCore.castMessage("Info: genericRead_BE4::read");
        return false;
    }
};

#endif

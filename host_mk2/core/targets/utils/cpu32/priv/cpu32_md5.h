#ifndef __MD5_CPU32_H__
#define __MD5_CPU32_H__

#include "../../../../bdmstuff.h"
#include "../../../../requests_cpu32.h"

// Etc
#include "../../../drivers/CPU32/generic/cpu32_md5.h"

// Originally implemented for f375 but you can likely use it on other CPU32. Either as is or by changing the SWSR pointer stored at offset 4
#include "../../../drivers/CPU32/specific/cpu32_f375_md5.h"

class CPU32_genmd5 : public virtual CPU32_requests {
    uint32_t driverBase;
public:
    explicit CPU32_genmd5(bdmstuff &p)
        : requests(p), CPU32_requests(p) {
        driverBase = 0;
    }

    bool upload(uint32_t address, bool silent = false) {
        driverBase = address;

        // Upload driver
        if ( !silent )
            core.castMessage("Info: Uploading md5 hash driver..");

        if ( fillDataBE4(address, CPU32_md5, sizeof(CPU32_md5)) == false ) {
            core.castMessage("Error: Unable to upload driver");
            return false;
        }

        return true;
    }

    bool upload(const uint8_t *data, size_t nBytes, uint32_t address, bool silent = false) {
        driverBase = address;

        // Upload driver
        if ( !silent )
            core.castMessage("Info: Uploading md5 hash driver..");

        if ( fillDataBE4(address, data, nBytes) == false ) {
            core.castMessage("Error: Unable to upload driver");
            return false;
        }

        return true;
    }

    bool hash(md5k_t *keys, uint32_t start, uint32_t length, bool silent = false) {
        uint16_t status;
        bool retVal;

        if ( keys == nullptr ) {
            core.castMessage("Error: md5 - Need a key buffer");
            return false;
        }

        core.queue  = writeAddressRegister(0, start);
        core.queue += writeDataRegister(0, length);
        core.queue += writeSystemRegister(0, driverBase);
        core.queue += targetStart();

        if ( !core.queue.send() ) {
            core.castMessage("Error: md5 - Unable to configure and/or start driver");
            return false;
        }

        if ( !silent )
            core.castMessage("Info: Hashing..");

        do {
            retVal = core.getStatus(&status);
            sleep_ms(25);
        } while ( retVal && status == RET_TARGETRUNNING );

        if ( status != RET_TARGETSTOPPED ) {
            core.castMessage("Error: md5 - Could not stop target");
            return false;
        }

        core.queue  = readAddressRegister(2);
        core.queue += readAddressRegister(3);
        core.queue += readAddressRegister(4);
        core.queue += readAddressRegister(5);

        if ( core.queue.send() == false ||
             core.getData(&((uint16_t *)keys)[0], TAP_DO_READREGISTER, 4, 0) == false ||
             core.getData(&((uint16_t *)keys)[2], TAP_DO_READREGISTER, 4, 1) == false ||
             core.getData(&((uint16_t *)keys)[4], TAP_DO_READREGISTER, 4, 2) == false ||
             core.getData(&((uint16_t *)keys)[6], TAP_DO_READREGISTER, 4, 3) == false)
        {
            core.castMessage("Error: Unable to retrieve flash information");
            return false;
        }

        // This needs to die
        keys->A = bs32(keys->A);
        keys->B = bs32(keys->B);
        keys->C = bs32(keys->C);
        keys->D = bs32(keys->D);

        return true;
    }
};

#endif

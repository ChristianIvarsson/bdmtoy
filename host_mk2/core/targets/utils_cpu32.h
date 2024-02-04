#ifndef __UTILS_CPU32_H__
#define __UTILS_CPU32_H__

// Target-specific driver.. stuff
#include "../bdmstuff.h"
#include "../requests_cpu32.h"
#include "../utils/crypto.h"

#include "utils_shared.h"
#include "partition_helper.h"

// Target drivers
#include "drivers/CPU32/generic/flash/genflash.h"
#include "drivers/CPU32/generic/md5/md5.h"

class cpu32_utils : public virtual requests_cpu32 {

    class md5 {
        cpu32_utils & utl;
        uint32_t driverBase;
    public:
        explicit md5(cpu32_utils &p)
            : utl(p) {
            driverBase = 0;
        }

        bool upload( uint32_t address, bool silent = false ) {

            driverBase = address;

            // Upload driver
            if ( !silent )
                utl.core.castMessage("Info: Uploading md5 hash driver..");

            if ( utl.fillDataBE4( address, CPU32_md5, sizeof(CPU32_md5) ) == false ) {
                utl.core.castMessage("Error: Unable to upload driver");
                return false;
            }

            return true;
        }

        bool hash( md5k_t *keys, uint32_t start, uint32_t length, bool silent = false ) {
            uint16_t status;
            bool retVal;

            if ( keys == nullptr ) {
                utl.core.castMessage("Error: md5 - Need a key buffer");
                return false;
            }

            utl.core.queue  = utl.writeAddressRegister( 0, start );
            utl.core.queue += utl.writeDataRegister( 0, length );
            utl.core.queue += utl.writeSystemRegister( 0, driverBase );
            utl.core.queue += utl.targetStart();

            if ( !utl.core.queue.send() ) {
                utl.core.castMessage("Error: md5 - Unable to configure and/or start driver");
                return false;
            }

            if ( !silent )
                utl.core.castMessage("Info: Hashing..");

            do {
                retVal = utl.core.getStatus( &status );
                sleep_ms( 25 );
            } while ( retVal && status == RET_TARGETRUNNING );

            if ( status != RET_TARGETSTOPPED ) {
                utl.core.castMessage("Error: md5 - Could not stop target");
                return false;
            }

            utl.core.queue  = utl.readAddressRegister( 2 );
            utl.core.queue += utl.readAddressRegister( 3 );
            utl.core.queue += utl.readAddressRegister( 4 );
            utl.core.queue += utl.readAddressRegister( 5 );

            if ( utl.core.queue.send() == false ||
                 utl.core.getData( &((uint16_t*)keys)[0], TAP_DO_READREGISTER, 4, 0 ) == false ||
                 utl.core.getData( &((uint16_t*)keys)[2], TAP_DO_READREGISTER, 4, 1 ) == false ||
                 utl.core.getData( &((uint16_t*)keys)[4], TAP_DO_READREGISTER, 4, 2 ) == false ||
                 utl.core.getData( &((uint16_t*)keys)[6], TAP_DO_READREGISTER, 4, 3 ) == false ) {
                 utl.core.castMessage("Error: Unable to retrieve flash information");
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

    class flashdrv {
        cpu32_utils & utl;
        uint32_t driverBase;
    public:
        explicit flashdrv(cpu32_utils &p)
            : utl(p) {
            driverBase = 0;
        }

        bool upload( uint32_t address ) {

            driverBase = address;

            // Upload driver
            utl.core.castMessage("Info: Uploading flash driver..");

            if ( utl.fillDataBE4( address, genDriver, sizeof(genDriver) ) == false ) {
                utl.core.castMessage("Error: Unable to upload driver");
                return false;
            }

            return true;
        }

        bool detect( flashid_t & id, uint32_t destination, uint32_t flashBase = 0 ) {
            uint16_t status;
            bool retVal;

            // Upload driver
            utl.core.castMessage("Info: Uploading detect driver..");

            if ( utl.fillDataBE4( destination, genDetect, sizeof(genDetect) ) == false ) {
                utl.core.castMessage("Error: Unable to upload driver");
                return false;
            }

            utl.core.queue  = utl.writeAddressRegister( 0, flashBase );
            utl.core.queue += utl.writeSystemRegister( 0, destination );
            utl.core.queue += utl.targetStart();

            if ( !utl.core.queue.send() ) {
                utl.core.castMessage("Error: detect - Unable to configure and/or start driver");
                return false;
            }

            utl.core.castMessage("Info: Waiting..");

            do {
                retVal = utl.core.getStatus( &status );
                sleep_ms( 25 );
            } while ( retVal && status == RET_TARGETRUNNING );

            if ( status != RET_TARGETSTOPPED ) {
                utl.core.castMessage("Error: detect - Could not stop target");
                return false;
            }


            uint16_t idTemp[ 8 ];
            utl.core.castMessage("Info: Retrieving flash info..");

            utl.core.queue  = utl.readDataRegister( 4 ); // MID
            utl.core.queue += utl.readDataRegister( 5 ); // DID / PID
            utl.core.queue += utl.readDataRegister( 6 ); // Size
            utl.core.queue += utl.readDataRegister( 7 ); // Type

            if ( utl.core.queue.send() == false ||
                utl.core.getData( &idTemp[0], TAP_DO_READREGISTER, 4, 0 ) == false || // MID
                utl.core.getData( &idTemp[2], TAP_DO_READREGISTER, 4, 1 ) == false || // DID / PID
                utl.core.getData( &idTemp[4], TAP_DO_READREGISTER, 4, 2 ) == false || // Size
                utl.core.getData( &idTemp[6], TAP_DO_READREGISTER, 4, 3 ) == false ){ // Type
                utl.core.castMessage("Error: Unable to retrieve flash information");
                return false;
            }

            uint32_t flashSize = *(uint32_t *) &idTemp[4];
            uint32_t flashType = *(uint32_t *) &idTemp[6];

            utl.core.castMessage("Info: MID    %04X", idTemp[0]);
            utl.core.castMessage("Info: DID    %04X", idTemp[2]);
            utl.core.castMessage("Info: base %06x"  , flashBase);
            utl.core.castMessage("Info: size %06X"  , flashSize); 

            switch ( flashType ) {
            case 1:  utl.core.castMessage("Info: Old H/W flash"); break;
            case 2:  utl.core.castMessage("Info: Modern toggle flash"); break;
            case 3:  utl.core.castMessage("Info: Disgusting Atmel flash.."); break;
            default: utl.core.castMessage("Error: Driver does not understand this flash"); return false;
            }

            id.MID = idTemp[0];
            id.DID = idTemp[2];

            return true;
        }

    };


protected:

    md5 md5;
    flashdrv flash;




public:

    explicit cpu32_utils(bdmstuff &p)
        : requests(p), requests_cpu32(p), md5(*this), flash(*this) {
            printf("cpu32_utils()\n");
    }









};

#endif

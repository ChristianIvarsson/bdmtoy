
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cstdlib>

#include "../../core/bdmstuff.h"

#include "tools.h"

class m_stuff : public bdmstuff {

public:
    void castProgress(int32_t) {

    }

    void castMessage(const char *str,...) {
        char tmp[256];
        va_list ap;
        va_start(ap, str);
        if (vsprintf(tmp, str, ap) > 0)
            printf("%s\n", tmp);
        va_end(ap);
    }

    ~m_stuff() {
        printf("m_stuff::~m_stuff()\n");
    }
};

m_stuff stuff;

static void printTargets(void)
{
    uint8_t strBuf[512];
    strBuf[sizeof(strBuf) - 1] = 0;
    int32_t nTargets = bdmstuff::numTargets;

    const target_t *target;

    printf("\nAvailable targets:\n");

    for ( int32_t i = 0; i < nTargets; i++ ) {
        size_t nameLen = 0;
        size_t infoLen = 0;
        memset( strBuf, ' ', sizeof(strBuf) - 1 );

        target = bdmstuff::info( i );

        if ( target->name != nullptr ) {
            nameLen = strlen( target->name );
            if ( nameLen > (sizeof(strBuf) - 1) )
                nameLen = sizeof(strBuf) - 1;
            if ( nameLen )
                memcpy( strBuf, target->name, nameLen );
        }

        if ( target->info ) {
            infoLen = strlen( target->info );
            if ( (nameLen + infoLen) > (sizeof(strBuf) - 1) )
                infoLen = (sizeof(strBuf) - 1) - nameLen;
            if ( infoLen )
                memcpy( &strBuf[ ((nameLen < 50) ? 50 : nameLen) ], target->info, infoLen );
        }

        strBuf[ ((nameLen < 50) ? 50 : nameLen) + infoLen ] = 0;

        printf("Target %2u: %s\n", i, strBuf);

        for ( uint32_t r = 0; r < target->regions; r++ ) {
            switch ( target->region[ r ].type ) {
            case opFlash:  printf("           Flash    %08llx : %uk\n", target->region[ r ].address , target->region[ r ].size / 1024); break;
            case opSRAM:   printf("           SRAM     %08llx : %uk\n", target->region[ r ].address , target->region[ r ].size / 1024); break;
            case opEEPROM: printf("           EEPROM   %08llx : %uk\n", target->region[ r ].address , target->region[ r ].size / 1024); break;
            default:       printf("           Unknown  %08llx : %uk\n", target->region[ r ].address , target->region[ r ].size / 1024); break;
            }
        }
        printf("\n");
    }
}

static bool dumpFlashTarget( int32_t idx, const char *fName ) {
    const target_t *info = bdmstuff::info( idx );
    for ( uint32_t idx = 0; idx < info->regions; idx++ ) {
        if ( info->region[ idx ].type == opFlash ||
             info->region[ idx ].type == opEEPROM ) {
            printf("Info: Match at index %u\n", idx);
            return ( stuff.read( idx ) && stuff.saveFile( fName ) );
        }
    }
    return false;
}

static bool writeFlashTarget( int32_t idx, const char *fName ) {
    const target_t *info = bdmstuff::info( idx );
    for ( uint32_t idx = 0; idx < info->regions; idx++ ) {
        if ( info->region[ idx ].type == opFlash ||
             info->region[ idx ].type == opEEPROM ) {
            printf("Info: Match at index %u\n", idx);
            return ( stuff.readFile( fName ) && stuff.write( idx ) );
        }
    }
    return false;
}

static bool dumpSramTarget( int32_t idx, const char *fName ) {
    const target_t *info = bdmstuff::info( idx );
    for ( uint32_t idx = 0; idx < info->regions; idx++ ) {
        if ( info->region[ idx ].type == opSRAM ) {
            printf("Info: Match at index %u\n", idx);
            return ( stuff.read( idx ) && stuff.saveFile( fName ) );
        }
    }
    return false;
}


static bool nukeTarget( int ) {
    printf("Implement me\n");
    return false;
}

static bool runWaitTarget( int32_t, const char *, const char* ) {
    printf("Implement me\n");
    return false;
}

static bool runSramTarget( int32_t, const char *, const char* ) {
    printf("Implement me\n");
    return false;
}

int main(int argc, char *argv[]) {

    numchar sc;
    int32_t target = 0xffff;

    printf("Version: %s\n", bdmstuff::version);

    if (argc < 2)
    {
        printf("\nCommands:\n"
               "    --targets                     - Dump list of targets\n"
               "    --target <num>                - Select target\n"
               "    --dump <file>                 - Dump target to file\n"
               "    --flash <file>                - Flash target from file\n"
               "    --runsram <file> <address>    - Upload blob to address and run it\n"
               "    --runwait <file> <address>    - Reset and run target, wait for it to enter bdm on its own, upload blob and then resume operation (Only target 7)\n"
               "    --nuke                        - Nuke hcs12 flash (Only target 6)\n");
        return 1;
    }

    // Print targets
    for (int i = 1; i < argc; i++) {
        if (strstr(argv[i], "--targets")) {
            printTargets();
            return 0;
        }
    }

    // Left in place for now. Not quite done with the conversion testing
/*
    for ( int i = 0; i < argc; i++ ) {
        sc = argv[ i ];
        if (sc.isDec<int32_t>()) printf("Dec: %d\n", sc.asDec<int32_t>());
        if (sc.isHex<uint32_t>()) printf("Hex: %x\n", (int32_t)sc.asHex<uint32_t>());
    }*/

    // Find target in arg list
    // (Nothing can be done without knowing which target to talk to)
    for (int i = 1; i < argc; i++) {
        const char *ptr = strstr(argv[i], "--target");
        int remainargs = (argc - 1) - i;
        if (ptr && !remainargs) {
            printf("You must also specify target number\n");
            return 1;
        }
        else if (ptr && remainargs) {
            sc = argv[i + 1];
            if (!sc.isDec<int32_t>()) {
                printf("Specified target number is malformed\n");
                return 1;
            }
            target = sc.asDec<int32_t>();
            break;
        }
        else if (!ptr && !remainargs) {
            printf("Please pass --target and decimal number\n");
            return 1;
        }
    }

    if (target < 0 || target >= bdmstuff::numTargets) {
        printf("Target number out of bounds!\n");
        return 1;
    }

    printf("Selected index %d ( %s )\n", target, bdmstuff::name( target ));
    
    if ( stuff.load( target ) == false ) {
        printf("Unable to load target for some unknown reason?");
        return 1;
    }

    for (int32_t i = 1; i < argc; i++)
    {
        int32_t remainargs = (argc - 1) - i;
        bool status = true;

        if (strstr(argv[i], "--nuke") && target == 7) {
            status = nukeTarget( i );
        }
        else if (strstr(argv[i], "--runwait")) {
            // Reset target and run it, wait for it to enter bdm/debug etc, upload code to specified address and return operation from last pc
            if ( remainargs < 2 ) {
                printf("You must specify filename and destination\n");
                return false;
            }
            status = runWaitTarget( target, argv[i + 1], argv[i + 2] );
        }
        else if (strstr(argv[i], "--runsram")) {
            if ( remainargs < 2 ) {
                printf("You must specify filename and destination\n");
                return false;
            }
            status = runSramTarget( target, argv[i + 1], argv[i + 2] );
        }
        else if (strstr(argv[i], "--dumpsram")) {
            if ( remainargs == 0 ) {
                printf("You must supply a filename\n");
                return 1;
            }
            status = dumpSramTarget( target, argv[i + 1] );
        }
        else if (strstr(argv[i], "--dump")) {
            if ( remainargs == 0 ) {
                printf("You must supply a filename\n");
                return 1;
            }
            status = dumpFlashTarget( target, argv[i + 1] );
        }
        else if (strstr(argv[i], "--flash")) {
            if ( remainargs == 0 ) {
                printf("You must supply a filename\n");
                return 1;
            }
            status = writeFlashTarget( target, argv[i + 1] );
        }

        if ( status == false ) {
            printf("Task failed\n");
            return 1;
        }
    }

    return 0;
}
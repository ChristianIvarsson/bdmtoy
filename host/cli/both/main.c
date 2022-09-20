#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include "../../../shared/enums.h"
#include "../../core/core.h"

#include "../../core/core_requests.h"
#include "../../core/core_requests_HCS12.h"

#include "../../core/targets/targets.h"

#include "../shared/str_tools.h"
#include "../shared/usb_glue.h"

static void castText(char *s)  { printf("%s\n", s); }
static void castprog(int prog) {}
static void callback()         { printf("Got a callback: %x\n", core_ReturnFaultStatus()); }

// Target 7
int32_t validFilename(char *filename)
{
    if (!filename)
        return -1;
    else if (strstr(filename, "-"))
        return -1;
    return 0;
}

uint32_t dumpFlash(uint32_t target, char *filename)
{
    if (validFilename(filename) != 0)
    {
        printf("Valid file name not supplied\n");
        exit(1);
    }
    core_DumpFLASH(target);
    runDamnit();

    if (core_ReturnFaultStatus() == 0)
        return wrk_writeFile(filename, core_TargetSizeFLASH(target));

    return core_ReturnFaultStatus();
}

uint32_t writeFlash(uint32_t target, char *filename)
{
    if (validFilename(filename) != 0)
    {
        printf("Valid file name not supplied\n");
        exit(1);
    }

    if (!wrk_openFile(filename))
    {
        printf("Could not open file!\n");
        exit(1);
    }

    core_FLASH(target, hack_ReturnBufferPtr());
    runDamnit();

    return core_ReturnFaultStatus();
}

uint32_t runSram(uint32_t target, char *filename, char *destination)
{
    uint32_t filelength = wrk_openFile(filename);
    if (!filelength) {
        printf("Could not open file!");
        exit(1);
    }
    if (!isHexNumber(destination)) {
        printf("Specified address is malformed\n");
        exit(1);
    }

    uint32_t address = toHexNumber(destination);
    printf("Address: %x\n", address);
    printf("Length : %u bytes\n", filelength);

    if (target == 7)
    {
        if (initSID95_MY0405())
        {
            printf("Could not init target\n");
            exit(1);
        }

        printf("Uploading data..\n");
        if ( TAP_FillDataBE2(address, filelength, hack_ReturnBufferPtr()) != RET_OK )
            core_castText("Failed to upload data");
        else if ( wrk_sendOneshot( HCS12_WritePC(address) ) != RET_OK )
            core_castText("Failed to set PC");
        else if ( wrk_sendOneshot( TAP_TargetStart() ) != RET_OK )
            core_castText("Failed to start target");
        else
        {
            core_castText("Target running... Waiting for it to stop again");
            if ( HCS12_WaitBDM() != RET_OK )
                core_castText("Failed while waiting for target to stop");
            else if (HCS12_PrintRegSummary() != RET_OK)
                core_castText("Could not fetch register values");
            else
                return 0;
        }
        return 0xFFFF;
    }
    else
    {
        printf("I don't know how to runwait this target!\n");
        exit(1);
    }
    return 0;
}

// Start target and wait patiently for it to enter debug mode again
// Currently only HCS12 (specifically SID MY 04/05)
uint32_t runWait(uint32_t target, char *filename, char *destination)
{
    uint32_t filelength = wrk_openFile(filename);
    if (!filelength) {
        printf("Could not open file!");
        exit(1);
    }
    if (!isHexNumber(destination)) {
        printf("Specified address is malformed\n");
        exit(1);
    }

    uint32_t address = toHexNumber(destination);
    printf("Address: %x\n", address);
    printf("Length : %u bytes\n", filelength);

    if (target == 7)
    {
        if (initBasic_HCS12())
        {
            printf("Could not bootstrap target\n");
            exit(1);
        }

        printf("Waiting for target to stop again...\n");
        if ( HCS12_WaitBDM() != RET_OK )
        {
            core_castText("Failed while waiting for target to stop");
            exit(1);
        }

        printf("Uploading data..\n");
        // Backup D, X, Y..
        uint16_t *ptr = wrk_requestData( HCS12_ReadD() );
        if (!ptr) return 0xFFFF;
        uint16_t D = ptr[2];
        ptr = wrk_requestData( HCS12_ReadX() );
        if (!ptr) return 0xFFFF;
        uint16_t X = ptr[2];
        ptr = wrk_requestData( HCS12_ReadY() );
        if (!ptr) return 0xFFFF;
        uint16_t Y = ptr[2];

        if ( TAP_FillDataBE2(address, filelength, hack_ReturnBufferPtr()) != RET_OK )
            core_castText("Failed to upload data");
        else if ( wrk_sendOneshot( HCS12_WriteD(D) ) != RET_OK )
            printf("Could not write reg\n");
        else if ( wrk_sendOneshot( HCS12_WriteX(X) ) != RET_OK )
            printf("Could not write reg\n");
        else if ( wrk_sendOneshot( HCS12_WriteY(Y) ) != RET_OK )
            printf("Could not write reg\n");
        else if ( wrk_sendOneshot( TAP_TargetStart() ) != RET_OK )
            core_castText("Failed to start target");
        else
        {
            printf("Target should be running now!\n");
            return 0;
        }
        return 0xFFFF;
    }
    else
    {
        printf("I don't know how to runwait this target!\n");
        exit(1);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int target = 0xffff;

    printf("%s\n", core_VersionString());

    if (argc < 2) {
        printf("Gotta pass some parameters, man!\n");
        exit(1);
    }

    // Find target in arg list
    // (Nothing can be done without knowing which target to talk to)
    for (int32_t i = 1; i < argc; i++) {
        
        char *ptr = strstr(argv[i], "--target");
        int remainargs = (argc - 1) - i;

        if (ptr && !remainargs) {
            printf("You must also specify target number\n");
            exit(1);
        }
        else if (ptr && remainargs) {
            ptr = argv[i+1];
            if (!isDecNumber(ptr))
            {
                printf("Specified target number is malformed\n");
                exit(1);
            }
            target = toDecNumber(ptr);
            break;
        }
        else if (!ptr && !remainargs) {
            printf("Please pass --target and decimal number\n");
            exit(1);
        }
    }

    if (target > core_NoTargets() || !target)
    {
        printf("Target number out of bounds!\n");
        exit(1);
    }

    if ( usb_test() != RET_OK ) {
        return 1;
    }
    // One is taken care of by usb_test() in usb_glue.c
    core_InstallProgress(&castprog);
    core_InstallCallback(&callback);
    core_InstallMessage(&castText);
/*
    if (secureEraseHCS12(4194000/2))
    {
        printf("Task failed\n");
        exit(1);
    }
*/
    for (int32_t i = 1; i < argc; i++) {
        
        // uint8_t *ptr = strstr(argv[i], "--target");
        int32_t remainargs = (argc - 1) - i;
        uint32_t retval = 0;
        // uint32_t doWhat = 0;

        if (strstr(argv[i], "--nuke") && target == 7) {
            retval = secureEraseHCS12(4194000/2);
        }

        // Reset target and run it, wait for it to enter bdm/debug etc, upload code to specified address and return operation from last pc
        else if (strstr(argv[i], "--runwait")) {
            if (remainargs < 2) {
                printf("You must specify filename and destination\n");
                exit(1);
            }
            retval = runWait((uint32_t)target, argv[i+1], argv[i+2]);
        }

        else if (strstr(argv[i], "--runsram")) {
            if (remainargs < 2) {
                printf("You must specify filename and destination\n");
                exit(1);
            }
            retval = runSram((uint32_t)target, argv[i+1], argv[i+2]);
        }

        else if (strstr(argv[i], "--dump")) {
            if (!remainargs) {
                printf("You must supply a filename\n");
                exit(1);
            }
            retval = dumpFlash((uint32_t)target, argv[i+1]);
        }
        else if (strstr(argv[i], "--flash")) {
            if (!remainargs) {
                printf("You must supply a filename\n");
                exit(1);
            }
            retval = writeFlash((uint32_t)target, argv[i+1]);
        }

        if (retval)
        {
            printf("Task failed\n");
            exit(1);
        }
    }

    return 0;
}
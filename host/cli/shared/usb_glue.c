#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>

#ifdef _WIN32
#include <windows.h>
#include "libusb.h"
#else
#include <unistd.h>
#include <sys/ioctl.h>
#include "../../libusb/libusb/libusb.h"
#endif

#include "../../../shared/enums.h"
#include "../../../shared/cmddesc.h"
#include "../../core/core.h"



#define NUM_INTERFACES   (2)

#define IN_EP            (1)
#define OUT_EP           (3)

static pthread_t thread_id;

static libusb_device_handle *handle = NULL;
static libusb_context *ctx = NULL;

static struct libusb_transfer *transfer_in = NULL;
static struct libusb_transfer *transfer_out = NULL;

static uint8_t in_buffer[ ADAPTER_BUFzOUT ]; // adapter -> host
static uint8_t out_buffer[ ADAPTER_BUFzIN ]; // host -> adapter

static volatile uint32_t keepRunning = 0;

static volatile uint32_t sendBusy = 0;


////////////////////////////////////////////////////////////////////
// Comm. handling

static void usbSendCallback(struct libusb_transfer *transfer) {
    sendBusy = 0;
    if ( transfer->status != 0 )
        printf("<USB send> Error code %s\n", libusb_error_name(transfer->status));
}

static void usbSendData(void *ptr, uint32_t toSend)
{
    int retStatus;

    while ( sendBusy ){}
    sendBusy = 1;

    memcpy(out_buffer, ptr, toSend);
    transfer_out->length = (int32_t)toSend;

    if ((retStatus = libusb_submit_transfer( transfer_out )) != 0)
        printf("<USB send> Error code %s\n", libusb_error_name(retStatus));
}

static void usbReceiveCallback(struct libusb_transfer *transfer)
{
    int retStatus;

    if ( transfer->status != 0 && transfer->status != LIBUSB_TRANSFER_CANCELLED )
        printf("<USB receive> Error code %s\n", libusb_error_name(transfer->status));

    if ( transfer->actual_length >= 0 )
        core_HandleRecData(transfer->buffer, (uint32_t)transfer->actual_length);

    if ((retStatus = libusb_submit_transfer( transfer )) != 0)
        printf("<USB receive> Error code %s\n", libusb_error_name(retStatus));
}

static void *usbAsyncThread(void *vargp)
{
    int retStatus;

    keepRunning = 1;

    while ( keepRunning ) {
        if ( (retStatus = libusb_handle_events_completed( ctx, NULL )) != 0 )
            printf("<USB evt loop> Error code %s\n", libusb_error_name(retStatus));
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////
// Initialisation

static uint32_t usb_OpenDevice()
{
    int res;

    if (libusb_init(&ctx) != 0) {
        printf("Could not initialise libusb\n");
        return RET_ABANDON;
    }

    // libUSB has a known race condition on windows.
    // It wont affect this code but it will throw assertion messages if any sort of logging is enabled
    libusb_set_option(ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_NONE);

    if ((handle = libusb_open_device_with_vid_pid(ctx, 0xFFFF, 0x0107)) == 0)
    {
        printf("Unable to open device.\n");
        return RET_ABANDON;
    }

#ifdef _WIN32
    libusb_set_auto_detach_kernel_driver(handle, 1);
#endif

    for (int i = 0; i < NUM_INTERFACES; i++)
    {
        if (libusb_kernel_driver_active(handle, i))
            libusb_detach_kernel_driver(handle, i);

        if ((res = libusb_claim_interface(handle, i)) != 0)
        {
            printf("Error claiming interface: %s\n", libusb_error_name(res));
            return RET_ABANDON;
        }
    }

    libusb_fill_bulk_transfer((transfer_in = libusb_alloc_transfer(0)),
                              handle,
                              LIBUSB_ENDPOINT_IN | IN_EP,
                              in_buffer,
                              ADAPTER_BUFzOUT,
                              usbReceiveCallback,
                              NULL,
                              0);

    libusb_submit_transfer(transfer_in);

    // Prep transfer but don't submit it
    libusb_fill_bulk_transfer((transfer_out = libusb_alloc_transfer(0)),
                              handle,
                              LIBUSB_ENDPOINT_OUT | OUT_EP,
                              out_buffer,
                              0,
                              usbSendCallback,
                              NULL,
                              0);

    return RET_OK;
}

uint32_t usb_test()
{
    int res;

    printf("\nInitialising USB..\n");

    if ((res = usb_OpenDevice()) != RET_OK)
    {
        printf("\nCould not connect to adapter\n");
        return res;
    }

    // Bring up asynchronous USB thread
    pthread_create(&thread_id, NULL, usbAsyncThread, NULL);

    // Install pointer
    core_InstallSendArray( usbSendData );

    waitms(2000);

    return RET_OK;
}

uint32_t usb_CleanUp()
{
    if ( keepRunning ) {

        keepRunning = 0;

        libusb_cancel_transfer( transfer_in );
        libusb_cancel_transfer( transfer_out );

        waitms( 100 );

        libusb_close( handle );
        pthread_join( thread_id, NULL );

        libusb_free_transfer( transfer_in );
        libusb_free_transfer( transfer_out );
    }

    if ( ctx != NULL )
        libusb_exit(ctx);

    transfer_out = NULL;
    transfer_in = NULL;
    handle = NULL;
    ctx = NULL;

    printf("USB shut down");

    return RET_OK;
}

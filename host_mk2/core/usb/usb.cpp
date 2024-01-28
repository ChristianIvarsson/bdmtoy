#include "usb.h"
#include "../bdmstuff.h"

// There's a couple of 0-length arrays in there.
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include "libusb.h"
#pragma GCC diagnostic pop
#else
#include "libusb.h"
#endif

#define NUM_INTERFACES   (2)

#define IN_EP            (1)
#define OUT_EP           (3)

static void usbReceiveCallback(struct libusb_transfer *transfer) {
    int retStatus;

    if ( transfer->status != 0 && transfer->status != LIBUSB_TRANSFER_CANCELLED )
        ((iUSB*)transfer->user_data)->core.castMessage("Error: usb::usbReceiveCallback() code %s\n", libusb_error_name(transfer->status));

    if ( transfer->actual_length > 0 ) {
        if ( (transfer->actual_length & 1) > 0 )
            ((iUSB*)transfer->user_data)->core.castMessage("Warning: usb::usbReceiveCallback() odd length!\n");
        ((iUSB*)transfer->user_data)->worker.receive( transfer->buffer, (uint32_t)transfer->actual_length );
    }

    if ((retStatus = libusb_submit_transfer( transfer )) != 0)
        ((iUSB*)transfer->user_data)->core.castMessage("Error: usb::usbReceiveCallback() code %s\n", libusb_error_name(retStatus));
}

static void usbSendCallback(struct libusb_transfer *transfer) {
    ((iUSB*)transfer->user_data)->sendBusy = false;
    if ( transfer->status != 0 )
        ((iUSB*)transfer->user_data)->core.castMessage("Error: usb::usbSendCallback() code %s\n", libusb_error_name(transfer->status));
}

bool usb::send(void *ptr, uint32_t toSend) {
    int retStatus;

    while ( interface.sendBusy ){}
    interface.sendBusy = 1;

    memcpy(out_buffer, ptr, toSend);
    // transfer_out->buffer = (unsigned char*)ptr;
    transfer_out->length = (int32_t)toSend;

    if ((retStatus = libusb_submit_transfer( transfer_out )) != 0) {
        core.castMessage("Error: usb::send() code %s", libusb_error_name(retStatus));
        return false;
    }

    return true;
}

void usb::usbThread() {

    int retStatus;

    threadRunning = true;
    threadStop = false;

    core.castMessage("Info: usb::usbThread()");

    while ( !threadStop ) {
        if ( (retStatus = libusb_handle_events_completed( ctx, NULL )) != 0 )
            core.castMessage("Error: usb::usbThread() code %s", libusb_error_name(retStatus));
    }

    // Can't use this
    // threadRunning = false;
}

bool usb::openDevice() {

    int res;

    core.castMessage("Info: usb::openDevice()");

    if (libusb_init(&ctx) != 0) {
        core.castMessage("Error: Could not initialise libusb");
        return false;
    }

    // libUSB has a known race condition on windows.
    // It wont affect this code but it will throw assertion messages if any sort of logging is enabled
    libusb_set_option(ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_NONE);

    if ((handle = libusb_open_device_with_vid_pid(ctx, 0xFFFF, 0x0107)) == nullptr)
    {
        core.castMessage("Error: Unable to open device");

        if ( ctx != nullptr )
            libusb_exit(ctx);
        ctx = nullptr;
        
        return false;
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
            core.castMessage("Error: Could not claim interface: %s", libusb_error_name(res));

            if ( handle != nullptr )
                libusb_close( handle );
            handle = nullptr;

            if ( ctx != nullptr )
                libusb_exit(ctx);
            ctx = nullptr;

            return false;
        }
    }

    libusb_fill_bulk_transfer((transfer_in = libusb_alloc_transfer(0)),
                              handle,
                              LIBUSB_ENDPOINT_IN | IN_EP,
                              in_buffer,
                              ADAPTER_BUFzOUT,
                              usbReceiveCallback,
                              this,
                              0);

    libusb_submit_transfer(transfer_in);

    // Prep transfer but don't submit it
    libusb_fill_bulk_transfer((transfer_out = libusb_alloc_transfer(0)),
                              handle,
                              LIBUSB_ENDPOINT_OUT | OUT_EP,
                              out_buffer,
                              0,
                              usbSendCallback,
                              this,
                              0);

    isConnected = true;

    return true;
}

bool usb::connect() {

    timeout tim;

    core.castMessage("Info: usb::connect()");

    disconnect();

    if ( !openDevice() )
        return false;

    try {
        usbThrd = std::thread(usbThread, this);
    } catch (...) {
        core.castMessage("Error: Exception thrown during USB threadspin");
        disconnect();
        return false;
    }

    tim.set( 4000 );

    // Wake up, damn you!
    while ( !threadRunning && !tim.outatime() ) {}

    if ( !threadRunning ) {
        core.castMessage("Error: USB thread not starting up");
        disconnect();
        return false;
    }

    return true;
}

bool usb::disconnect() {

    if ( !isConnected && !threadRunning )
        return true;

    core.castMessage("Info: usb::disconnect()");

    if ( transfer_in != nullptr )
        libusb_cancel_transfer( transfer_in );
    if ( transfer_out != nullptr )
        libusb_cancel_transfer( transfer_out );
    
    threadStop = true;

    sleep_ms( 100 );

    if ( handle != nullptr )
        libusb_close( handle );

    if ( threadRunning ) {
        try {
            usbThrd.join();
        } catch (...) {
            core.castMessage("Error: Exception while joining thread");
        }
    }

    if ( transfer_in != nullptr )
        libusb_free_transfer( transfer_in );
    if ( transfer_out != nullptr )
        libusb_free_transfer( transfer_out );

    if ( ctx != nullptr )
        libusb_exit(ctx);

    transfer_out = nullptr;
    transfer_in = nullptr;
    handle = nullptr;
    ctx = nullptr;

    threadRunning = false;
    isConnected = false;

    interface.sendBusy = false;

    return true;
}

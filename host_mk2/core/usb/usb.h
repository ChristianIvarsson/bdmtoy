#ifndef __USB_H__
#define __USB_H__

#include <cstdio>
#include <cstdint>
#include <thread>

// Buffer sizes
#include "../../../shared/cmddesc.h"

class bdmstuff;
class bdmworker;

struct libusb_context;
struct libusb_device;
struct libusb_device_handle;

// libUSB needs static functions for callbacks
class iUSB {
public:
    explicit iUSB( bdmstuff & par, bdmworker & wrk )
        : core(par), worker(wrk) {}

    volatile bool sendBusy = false;

    bdmstuff  & core;
    bdmworker & worker;
};

class usb : private iUSB {

    uint8_t in_buffer[ ADAPTER_BUFzOUT ]; // adapter -> host
    uint8_t out_buffer[ ADAPTER_BUFzIN ]; // host -> adapter

    std::thread usbThrd;
    void usbThread();

    libusb_device_handle   *handle       = nullptr;
    libusb_context         *ctx          = nullptr;
    struct libusb_transfer *transfer_in  = nullptr;
    struct libusb_transfer *transfer_out = nullptr;

    bool          isConnected   = false;
    volatile bool threadRunning = false;
    volatile bool threadStop    = false;
    
    bool openDevice();

    iUSB & interface;

protected:
    bool send(void *, uint32_t);

public:
    explicit usb( bdmstuff & parent, bdmworker & worker )
    : iUSB( parent, worker ), interface(*this) {}
    ~usb() { disconnect(); }

    bool connect();
    bool disconnect();
};

#endif

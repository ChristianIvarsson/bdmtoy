#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "../../../shared/enums.h"
#include "../../../shared/cmddesc.h"
#include "../../core/core.h"


#ifdef _WIN32

#include <windows.h>
#include "libusb.h"

static HANDLE thread_id;

#else

#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include "../../libusb/libusb/libusb.h"

static pthread_t thread_id;

#endif

static struct libusb_transfer *transfer_out;
static struct libusb_transfer *transfer_in;
static libusb_device_handle *handle;
static libusb_context *ctx = NULL;

static uint8_t in_buffer[ADAPTER_BUFzOUT];

// Thread

static volatile uint32_t keepRunning = 1;


static void cb_in(struct libusb_transfer *transfer)
{
	core_HandleRecData(transfer->buffer, transfer->actual_length);
	libusb_submit_transfer(transfer_in);
}

#if _WIN32
DWORD WINAPI usb_AsyncThread(void* data)
#else
static void *usb_AsyncThread(void *vargp) 
#endif
{
	printf("USB async. thread alive\n");

	while (keepRunning)
	{
  		libusb_handle_events(ctx);
  		// libusb_handle_events_completed(ctx, NULL);
	}

	printf("Async. thread going down\n");
	return NULL;
} 




static uint32_t usb_OpenDevice()
{
	// int kernelDriverDetached     = 0;
	int res;

	if ( libusb_init(&ctx)!= 0 )
	{
		printf("Could not initialize libusb.\n");
		return 0xFFFF;
	}



#ifdef _WIN32
	// This option should be set before calling libusb_init()
	// LIBUSB_OPTION_USE_USBDK

	// Or here according to their github... fucking libusb
	// libusb_set_option(ctx, LIBUSB_OPTION_USE_USBDK);
#endif
	libusb_set_debug(NULL, 3);
  	if ((handle = libusb_open_device_with_vid_pid(ctx, 0xFFFF, 0x0107)) == 0)
  	{
    	printf("Unable to open device.\n");
    	return 1;
  	}

#ifndef _WIN32
  	if (libusb_kernel_driver_active(handle, 0))
  	{
    	// res = 
		libusb_detach_kernel_driver(handle, 0);
    	/*
		if (res < 0)
		{
			printf("Error detaching kernel driver.\n");
			return 1;
		}*/
		/*if (res == 0)
    	{
      		// kernelDriverDetached = 1;
    	}
    	else
    	{
      		printf("Error detaching kernel driver.\n");
      		return 1;
    	}*/
	}

#else
	libusb_set_auto_detach_kernel_driver(handle, 1);
#endif




	/*
	
	if (libusb_set_configuration(handle, 1) < 0)
	{
		printf("Error setting config\n");
	}


	*/



	// waitms(1000);


  	// res = libusb_claim_interface(handle, 0);
  	
	for (int if_num = 0; if_num < 2; if_num++) {
		if (libusb_kernel_driver_active(handle, if_num)) {
			libusb_detach_kernel_driver(handle, if_num);
		}
		res = libusb_claim_interface(handle, if_num);
		if (handle < 0) {
			fprintf(stderr, "Error claiming interface: %s\n",
				libusb_error_name(res));
			return 1;
		}
	}

	/*if (libusb_claim_interface(handle, 0) != 0)
  	{
    	fprintf(stderr, "Error claiming interface.\n");
    	return 1;
  	}*/

	transfer_in = libusb_alloc_transfer(0);

  	libusb_fill_bulk_transfer(transfer_in, handle, LIBUSB_ENDPOINT_IN | 1,
  		in_buffer, ADAPTER_BUFzOUT,
  		(libusb_transfer_cb_fn)&cb_in,
		NULL,
		0);

  	libusb_submit_transfer(transfer_in);

  	return RET_OK;
}

// static unsigned char data[8192];

static void usb_SendArr(void *ptr, uint32_t noBytes)
{
	int numBytes;

  	if ( libusb_bulk_transfer(handle,
						      LIBUSB_ENDPOINT_OUT | 3,
		                      (unsigned char *)ptr,
		                      noBytes,
							  &numBytes,
		                      2000       ) != 0 ||
		 noBytes != numBytes  )
  	{
    	printf("Error sending message to device\n");
  	}
}

uint32_t usb_test()
{
	int res;

	printf("\nInitializing USB..\n");

	res = usb_OpenDevice();

	if ( res != RET_OK )
	{
		printf("\nCould not connect to adapter\n");
		return res;
	}

	// Bring up asynchronous USB thread
#ifdef _WIN32
	thread_id = CreateThread(NULL, 0, usb_AsyncThread, NULL, 0, NULL);
#else
    pthread_create(&thread_id, NULL, usb_AsyncThread, NULL);
#endif


    // Install pointer
    core_InstallSendArray(&usb_SendArr);

	waitms(1000);
	waitms(1000);
    // sleep(1);

    return RET_OK;
}

uint32_t usb_CleanUp()
{
	keepRunning = 0;

#ifdef _WIN32
	TerminateThread(thread_id, NULL);
#else
	pthread_join(thread_id, NULL);
#endif

	return RET_OK;
}


APP=debugtool
CFLAGS= -pthread -O1

ifeq ($(OS),Windows_NT)
CC="x86_64-w64-mingw32-gcc.exe"
LDFLAGS= -m64 -L../shared/libs/win/libusb-MinGW-x64/lib -llibusb-1.0
CFLAGS+= -m64
else
CC=gcc
LDFLAGS=-lusb-1.0 -lpthread
endif

.PHONY: all clean

all: BUILDMSG $(APP)

BUILDMSG:
	@echo Building..

USBOBJS=\
	build/usb_glue.o

APPOBJS=\
	$(USBOBJS) \
	build/main.o \
	build/str_tools.o \
	build/core.o \
	build/core.o \
	build/core_debug.o \
	build/core_requests.o \
	build/core_requests_CPU32.o \
	build/core_requests_HCS12.o \
	build/core_requests_MPC.o \
	build/core_requests_JTAG.o \
	build/core_worker.o \
	build/utils.o \
	build/utl_md5.o \
	build/generic.o \
	build/BMW_cluster.o \
	build/hcs12.o \
	build/Trionic.o \
	build/mpc562.o \
	build/mpc5566.o \
	build/68hc08_uart.o


build/main.o:   main.c 
	@$(CC) $(CFLAGS) -Wall $< -c -o $@ 

build/str_tools.o:   ../shared/str_tools.c 
	@$(CC) $(CFLAGS) -Wall $< -c -o $@

build/usb_glue.o:   ../shared/usb_glue.c 
	@$(CC) $(CFLAGS) -Wall $< -c -o $@

# Core
build/core.o:   ../../core/core.c
	@$(CC) $(CFLAGS) -Wall $< -c -o $@
build/core_debug.o:   ../../core/core_debug.c
	@$(CC) $(CFLAGS) -Wall $< -c -o $@
build/core_requests.o:   ../../core/core_requests.c 
	@$(CC) $(CFLAGS) -Wall $< -c -o $@
build/core_requests_CPU32.o:   ../../core/core_requests_CPU32.c 
	@$(CC) $(CFLAGS) -Wall $< -c -o $@
build/core_requests_HCS12.o:   ../../core/core_requests_HCS12.c 
	@$(CC) $(CFLAGS) -Wall $< -c -o $@
build/core_requests_MPC.o:   ../../core/core_requests_MPC.c 
	@$(CC) $(CFLAGS) -Wall $< -c -o $@
build/core_requests_JTAG.o:   ../../core/core_requests_JTAG.c 
	@$(CC) $(CFLAGS) -Wall $< -c -o $@
build/core_worker.o:   ../../core/core_worker.c 
	@$(CC) $(CFLAGS) -Wall $< -c -o $@

# Utils
build/utils.o:       ../../core/utils/utils.c
	@$(CC) $(CFLAGS) -Wall $< -c -o $@
build/utl_md5.o:       ../../core/utils/utl_md5.c
	@$(CC) $(CFLAGS) -Wall $< -c -o $@

# Target code
build/generic.o:       ../../core/targets/generic.c 
	@$(CC) $(CFLAGS) -Wall $< -c -o $@
build/BMW_cluster.o:   ../../core/targets/BMW_cluster.c 
	@$(CC) $(CFLAGS) -Wall $< -c -o $@
build/hcs12.o:         ../../core/targets/hcs12.c 
	@$(CC) $(CFLAGS) -Wall $< -c -o $@
build/Trionic.o:       ../../core/targets/Trionic.c 
	@$(CC) $(CFLAGS) -Wall $< -c -o $@
build/mpc562.o:       ../../core/targets/mpc562.c 
	@$(CC) $(CFLAGS) -Wall $< -c -o $@
build/mpc5566.o:       ../../core/targets/mpc5566.c 
	@$(CC) $(CFLAGS) -Wall $< -c -o $@
build/68hc08_uart.o:       ../../core/targets/68hc08_uart.c 
	@$(CC) $(CFLAGS) -Wall $< -c -o $@


$(APP): $(APPOBJS)
	@$(CC) -o $@ $^ $(LDFLAGS)

clean:
	@rm -f build/*.o
	@rm -f $(APP)
	@rm -f $(APP).exe

CC=gcc
CXX=g++
SIZE=size

APP=cmfigen

CPPFLAGS=-O1 -pthread -Wall -Wpedantic -Werror -std=c++11 -DCMFI_COMPARE
LDFLAGS=-lusb-1.0 -lpthread

ifeq ($(OS),Windows_NT)
LIBUSBVER=MinGW64
LDFLAGS+= -m64 -L../../../host_mk2/apps/cli/shared/libs/win/$(LIBUSBVER)/static
CPPFLAGS+= -m64 -I../../../host_mk2/apps/cli/shared/libs/win/include
APP=cmfigen.exe
endif

.PHONY: all clean

all: BUILDMSG $(APP)

BUILDMSG:
	@echo Building..

TARGOBJS=\
	build/target_trionic.o \
	build/target_cpu32.o

APPOBJS=\
	build/main.o \
	build/bdmstuff.o \
	build/bdmworker.o \
	build/usb.o \
	build/printdata.o \
	$(TARGOBJS)

build/main.o:   main.cpp
	@$(CXX) $(CPPFLAGS) $< -c -o $@ 

build/printdata.o:   printdata.cpp
	@$(CXX) $(CPPFLAGS) $< -c -o $@ 

# Well this didn't go according to plan...
build/bdmstuff.o:   ../../../host_mk2/core/bdmstuff.cpp
	@$(CXX) $(CPPFLAGS) $< -c -o $@ 

build/bdmworker.o:   ../../../host_mk2/core/bdmworker.cpp
	@$(CXX) $(CPPFLAGS) $< -c -o $@ 

build/usb.o:   ../../../host_mk2/core/usb/usb.cpp
	@$(CXX) $(CPPFLAGS) $< -c -o $@ 


build/target_trionic.o:   ../../../host_mk2/core/targets/target_trionic.cpp
	@$(CXX) $(CPPFLAGS) $< -c -o $@ 

build/target_cpu32.o:   ../../../host_mk2/core/targets/target_cpu32.cpp
	@$(CXX) $(CPPFLAGS) $< -c -o $@ 



$(APP): $(APPOBJS)
	@$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	@rm -f build/*.o
	@rm -f $(APP)
	@rm -f $(APP).exe

CC=gcc
CXX=g++
SIZE=size

APP=debugtool
# -Wno-error=unused-variable

CFLAGS=-O1 -pthread -Wno-unused-function -Wno-unused-variable 
CPPFLAGS=-O1 -pthread -Wall -Wpedantic -Werror -std=c++11
LDFLAGS=-lusb-1.0 -lpthread

ifeq ($(OS),Windows_NT)
LIBUSBVER=MinGW64
LDFLAGS+= -m64 -L../shared/libs/win/$(LIBUSBVER)/static
CFLAGS+= -m64 -I../shared/libs/win/include
CPPFLAGS+= -m64 -I../shared/libs/win/include
APP=debugtool.exe
endif

.PHONY: all clean

all: BUILDMSG $(APP)

BUILDMSG:
	@echo Building..


TARGOBJS=\
	build/target_trionic.o \
	build/target_cpu32.o \
	build/target_helper.o

APPOBJS=\
	build/main.o \
	build/bdmstuff.o \
	build/bdmworker.o \
	build/usb.o \
	$(TARGOBJS)

build/main.o:   main.cpp
	@$(CXX) $(CPPFLAGS) $< -c -o $@ 



build/bdmstuff.o:   ../../../core/bdmstuff.cpp
	@$(CXX) $(CPPFLAGS) $< -c -o $@ 

build/bdmworker.o:   ../../../core/bdmworker.cpp
	@$(CXX) $(CPPFLAGS) $< -c -o $@ 

build/usb.o:   ../../../core/usb/usb.cpp
	@$(CXX) $(CPPFLAGS) $< -c -o $@ 



build/target_trionic.o:   ../../../core/targets/target_trionic.cpp
	@$(CXX) $(CPPFLAGS) $< -c -o $@ 

build/target_cpu32.o:   ../../../core/targets/target_cpu32.cpp
	@$(CXX) $(CPPFLAGS) $< -c -o $@ 

build/target_helper.o:   ../../../core/targets/target_helper.cpp
	@$(CXX) $(CPPFLAGS) $< -c -o $@ 







$(APP): $(APPOBJS)
	@$(CXX) -o $@ $^ $(LDFLAGS)
	@$(SIZE) -A $(APP)

clean:
	@rm -f build/*.o
	@rm -f $(APP)
	@rm -f $(APP).exe

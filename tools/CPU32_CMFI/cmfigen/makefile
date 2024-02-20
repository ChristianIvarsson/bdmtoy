CC=gcc
CXX=g++
SIZE=size

APP=cmfigen

CPPFLAGS=-O1 -Wall -Wpedantic -Werror -std=c++11
LDFLAGS=

ifeq ($(OS),Windows_NT)
APP=cmfigen.exe
endif

.PHONY: all clean

all: BUILDMSG $(APP)

BUILDMSG:
	@echo Building..

APPOBJS=\
	build/main.o \
	build/printdata.o

build/main.o:   main.cpp
	@$(CXX) $(CPPFLAGS) $< -c -o $@ 

build/printdata.o:   printdata.cpp
	@$(CXX) $(CPPFLAGS) $< -c -o $@ 


$(APP): $(APPOBJS)
	@$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	@rm -f build/*.o
	@rm -f $(APP)
	@rm -f $(APP).exe

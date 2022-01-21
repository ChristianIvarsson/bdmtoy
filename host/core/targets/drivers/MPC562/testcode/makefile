NAME    = MAIN

CC      = powerpc-eabi-gcc
AS      = powerpc-eabi-as

LINKER_SCRIPT=ls.ld

ASFLAGS = -mppc -mbig-endian -memb -mregnames
LDFLAGS = -mcpu=powerpc -nostdlib -Wl,-s -Wl,-n -Xlinker --gc-sections -T$(LINKER_SCRIPT) -Wl,-Map=$(basename $@).map

ifeq ($(OS),Windows_NT)
   RM = del /Q
   FixPath = $(subst /,\,$1)
else
   ifeq ($(shell uname), Linux)
      RM = rm -f
      FixPath = $1
   endif
endif

all:  out/testdriver.bin

out/testdriver.o: testdriver.s
	$(AS) $(ASFLAGS)  $< -o $@

out/testdriver.bin:    out/testdriver.o
	$(CC) $(LDFLAGS) -o $@ out/testdriver.o

	copy out\testdriver.bin mpc562test.bin
	bin2header mpc562test.bin
	del out\*.o
	copy mpc562test.bin.h ..\..\mpc562test.h
	del mpc562test.bin.h
	del mpc562test.bin
.PHONY: clean
clean:
	@$(RM) $(call FixPath,out/*)
ifeq ($(OS),Windows_NT)
	del *.bin, *.srec
endif

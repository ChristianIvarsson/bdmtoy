NAME    = c39eeprom

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

all:  out/c39eeprom.bin

out/c39eeprom.o: c39eeprom.s
	$(AS) $(ASFLAGS)  $< -o $@

out/c39eeprom.bin:    out/c39eeprom.o
	$(CC) $(LDFLAGS) -o $@ out/c39eeprom.o

	copy out\c39eeprom.bin c39eeprom.bin
	bin2header c39eeprom.bin
	del out\*.o
	copy c39eeprom.bin.h ..\..\c39eeprom.h
	del c39eeprom.bin.h
	del c39eeprom.bin
.PHONY: clean
clean:
	@$(RM) $(call FixPath,out/*)
ifeq ($(OS),Windows_NT)
	del *.bin, *.srec
endif

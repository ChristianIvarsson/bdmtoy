NAME    = md5

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

all:  out/ppcmd5.bin

out/ppcmd5.o: md5.s
	$(AS) $(ASFLAGS)  $< -o $@

out/ppcmd5.bin:    out/ppcmd5.o
	$(CC) $(LDFLAGS) -o $@ out/ppcmd5.o

	copy out\ppcmd5.bin ppcmd5.bin
	bin2header ppcmd5.bin
	del out\*.o
	copy ppcmd5.bin.h ..\..\ppcmd5.h
	del ppcmd5.bin.h
	del ppcmd5.bin
.PHONY: clean
clean:
	@$(RM) $(call FixPath,out/*)
ifeq ($(OS),Windows_NT)
	del *.bin, *.srec
endif

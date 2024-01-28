CC  = m68k-elf-gcc
AS  = m68k-elf-as

.PHONY: all clean

ASFLAGS = -mcpu32 --register-prefix-optional
LDFLAGS = -mcpu32 -nostdlib -Wl,-s -Wl,-n -Tcfg.ld -Wl,-Map=$(basename $@).map

all:  out/TxDriver.bin 

out/flash.o: flash.s
	@$(AS) $(ASFLAGS)  $< -o $@
	
out/TxDriver.bin: out/flash.o
	@$(CC) $(LDFLAGS) -o $@ out/flash.o

clean:
	@rm -f out/*.o
	@rm -f out/*.bin
	@rm -f out/*.map

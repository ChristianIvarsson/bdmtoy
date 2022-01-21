NAME    = test

CC      = m68hc12-ld
AS      = m68hc12-as

CPU_VARIANT=m68hcs12
LINKER_SCRIPT=map.ld
ASFLAGS = -m68hcs12 -S
LDFLAGS = -S -mm68hc12elf -e 0x2000 -nostdlib   -T$(LINKER_SCRIPT) 

ifeq ($(OS),Windows_NT)
   RM = del /Q
   FixPath = $(subst /,\,$1)
else
   ifeq ($(shell uname), Linux)
	  RM = rm -f
	  FixPath = $1
   endif
endif

all:  out/eeprom.elf

out/eeprom.o: eeprom.s
	$(AS) $(ASFLAGS)  $< -o $@

out/eeprom.elf:    out/eeprom.o
	$(CC) $(LDFLAGS) -o $@ out/eeprom.o
	m68hc12-objcopy -O binary out/eeprom.elf out/eeprom.bin

.PHONY: clean
clean:

	@$(RM) $(call FixPath,out/*)

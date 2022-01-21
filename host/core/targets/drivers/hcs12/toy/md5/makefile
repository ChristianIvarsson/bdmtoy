NAME    = test

CC      = m68hc12-ld
AS      = m68hc12-as

CPU_VARIANT=m68hc12
LINKER_SCRIPT=map.ld
ASFLAGS = -m68hc12 -S
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

all:  out/startup.o out/test.elf

out/startup.o: start.s
	$(AS) $(ASFLAGS)  $< -o $@

out/test.o: md5.s
	$(AS) $(ASFLAGS)  $< -o $@

out/test.elf:    out/test.o
	$(CC) $(LDFLAGS) -o $@ out/test.o
	m68hc12-objcopy -O binary out/test.elf out/test.bin

.PHONY: clean
clean:

	@$(RM) $(call FixPath,out/*)

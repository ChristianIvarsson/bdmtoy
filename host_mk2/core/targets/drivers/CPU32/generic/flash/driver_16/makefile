NAME=driver_16bit

CC=m68k-elf-gcc
AS=m68k-elf-as
SIZE=m68k-elf-size
OBJCOPY=m68k-elf-objcopy

ASFLAGS= -mcpu32 --register-prefix-optional

.PHONY: all clean

all : $(NAME)

OBJS=\
	out/flash.o \
	out/flash_toggle.o \
	out/flash_paged.o \
	out/flash_og.o

out/flash.o: flash.s
	@$(AS) $(ASFLAGS)  $< -o $@

out/flash_toggle.o: flash_toggle.s
	@$(AS) $(ASFLAGS)  $< -o $@
out/flash_paged.o: flash_paged.s
	@$(AS) $(ASFLAGS)  $< -o $@
out/flash_og.o: flash_og.s
	@$(AS) $(ASFLAGS)  $< -o $@


$(NAME) : $(OBJS)
	@$(CC) -o out/$(NAME).elf $^ -mcpu32 -nostdlib -Wl,-s -Wl,-n -Xlinker --gc-sections -Tlink.ld -Wl,-Map=out/$(basename $@).map
	@$(OBJCOPY) -O binary out/$(NAME).elf out/$(NAME).bin
	@$(OBJCOPY) -O ihex out/$(NAME).elf out/$(NAME).hex
	@$(SIZE) -A out/$(NAME).elf

clean:
	@rm -f out/*.bin
	@rm -f out/*.hex
	@rm -f out/*.elf
	@rm -f out/*.o
	@rm -f out/*.map

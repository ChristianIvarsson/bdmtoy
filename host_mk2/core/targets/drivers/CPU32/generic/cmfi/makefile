NAME=cpu32mcfi

CC=m68k-elf-gcc

AS=m68k-elf-as
LD=m68k-elf-ld
SIZE=m68k-elf-size
OBJCOPY=m68k-elf-objcopy

ASFLAGS= -mcpu32 --register-prefix-optional
CFLAGS= -mcpu32 -std=gnu99 -Werror -Wall -Wpedantic -O1 -fomit-frame-pointer

.PHONY: all clean

all : $(NAME)

OBJS=\
	out/entry.o \
	out/cmfi.o

out/entry.o: entry.s
	@$(AS) $(ASFLAGS) $< -o $@

out/cmfi.o: cmfi.c
	@$(CC) $(CFLAGS) $< -c -o $@

$(NAME) : $(OBJS)
	@$(CC) -o out/$(NAME).elf $^ -mcpu32 -nostdlib -Tlink.ld -Wl,-Map=out/$(basename $@).map
	@$(OBJCOPY) -O binary out/$(NAME).elf out/$(NAME).bin
	@$(OBJCOPY) -O ihex out/$(NAME).elf out/$(NAME).hex
	@$(SIZE) -A out/$(NAME).elf

clean:
	@rm -f out/*.bin
	@rm -f out/*.hex
	@rm -f out/*.elf
	@rm -f out/*.o
	@rm -f out/*.map

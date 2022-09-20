
# I use several gcc toolchains depending on what I do. You should be able to just comment out this and use the default
# Keep in mind that -O2 us quite agressive and has caused me a lot of trouble with some. You might want to go down to -O1 at the expense of a slightly slower adapter
GCCPATH="D:/ARM_GCC/6 2017-q2-update/bin/"
BASENAME=arm-none-eabi

CC=$(GCCPATH)$(BASENAME)-gcc
CPP=$(GCCPATH)$(BASENAME)-c++
LD=$(GCCPATH)$(BASENAME)-ld
OBJCOPY=$(GCCPATH)$(BASENAME)-objcopy
SIZE=$(GCCPATH)$(BASENAME)-size
GDB=$(GCCPATH)$(BASENAME)-gdb

TARGPROC=-mcpu=cortex-m3 -mthumb

SHAREDC=-ffunction-sections -O2 -Wall -nostartfiles -g3 -gdwarf-2

# Include development makefile if present
-include ../.private/firmdevflags

#  -DBIGBOARD
# Some of the library code is using if flags
CODEFLAGS=-DSTM32F103C8 -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER -D__ASSEMBLY__ $(DEVFLAGS) 

HEADERPATHS=\
	-ISyssup/CMSIS/Device/ST/STM32F10x \
	-ISyssup/STM32F10x_StdPeriph_Driver/inc \
	-ISyssup/CMSIS/Include \
	-ISyssup/CMSIS \
	-ISyssup/CMSIS/Device \
	-IUSB \
	-ISyssup/STM32F10x_StdPeriph_Driver \
	-ISyssup/STM32_USB-FS-Device_Driver/inc \
	-ISyssup/CMSIS/Device/ST \
	-ISyssup/STM32_USB-FS-Device_Driver

CFLAGS= $(TARGPROC) $(SHAREDC) -std=gnu99 $(CODEFLAGS) $(HEADERPATHS)

.PHONY: all flash debug

all : firmware

flash : all
debug : startdebug

TAPOBJS=\
	obj/TAP_shared.o \
	obj/TAP_lowlev.o \
	obj/BDMs_high.o \
	obj/BDMs_low.o \
	obj/BDMn_high.o \
	obj/BDMn_low.o \
	obj/BDMo_high.o \
	obj/BDMo_low.o \
	obj/JTAG_high.o \
	obj/NEXUS_high.o \
	obj/UARTMON_high.o

SYSOBJS=\
	obj/startup_stm32f10x_md.o \
	obj/system_stm32f10x.o \
	obj/misc.o \
	obj/stm32f10x_adc.o \
	obj/stm32f10x_bkp.o \
	obj/stm32f10x_can.o \
	obj/stm32f10x_cec.o \
	obj/stm32f10x_crc.o \
	obj/stm32f10x_dac.o \
	obj/stm32f10x_dbgmcu.o \
	obj/stm32f10x_dma.o \
	obj/stm32f10x_exti.o \
	obj/stm32f10x_flash.o \
	obj/stm32f10x_fsmc.o \
	obj/stm32f10x_gpio.o \
	obj/stm32f10x_i2c.o \
	obj/stm32f10x_iwdg.o \
	obj/stm32f10x_pwr.o \
	obj/stm32f10x_rcc.o \
	obj/stm32f10x_rtc.o \
	obj/stm32f10x_sdio.o \
	obj/stm32f10x_spi.o \
	obj/stm32f10x_tim.o \
	obj/stm32f10x_usart.o \
	obj/stm32f10x_wwdg.o

USBOBJS=\
	obj/usb_core.o \
	obj/usb_init.o \
	obj/usb_int.o \
	obj/usb_mem.o \
	obj/usb_regs.o \
	obj/usb_sil.o \
	obj/hw_config.o \
	obj/stm32_it.o \
	obj/usb_desc.o \
	obj/usb_istr.o \
	obj/usb_prop.o \
	obj/usb_pwr.o

ALLOBJS=\
	obj/main.o \
	obj/SupportFuncs.o \
	obj/syscalls.o \
	obj/printf.o \
	$(SYSOBJS) \
	$(USBOBJS) \
	$(TAPOBJS)


# Interface objects
obj/TAP_shared.o : TAP/TAP_shared.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/TAP_lowlev.o : TAP/TAP_lowlev.s
	@$(CC) $(CFLAGS) -c $< -o $@
obj/BDMs_high.o : TAP/BDM_HC12/BDMs_high.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/BDMs_low.o : TAP/BDM_HC12/BDMs_low.s
	@$(CC) $(CFLAGS) -c $< -o $@
obj/BDMn_high.o : TAP/BDM_NEW/BDMn_high.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/BDMn_low.o : TAP/BDM_NEW/BDMn_low.s
	@$(CC) $(CFLAGS) -c $< -o $@
obj/BDMo_high.o : TAP/BDM_OLD/BDMo_high.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/BDMo_low.o : TAP/BDM_OLD/BDMo_low.s
	@$(CC) $(CFLAGS) -c $< -o $@
obj/JTAG_high.o : TAP/JTAG/JTAG_high.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/NEXUS_high.o : TAP/NEXUS/NEXUS_high.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/UARTMON_high.o : TAP/UART_MON/UARTMON_high.c
	@$(CC) $(CFLAGS) -c $< -o $@


# Misc / main code
obj/main.o : main.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/SupportFuncs.o : SupportFuncs.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/printf.o : Syssup/stdio/printf.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/syscalls.o : Syssup/syscalls/syscalls.c
	@$(CC) $(CFLAGS) -c $< -o $@


# System objects
obj/startup_stm32f10x_md.o : Syssup/CMSIS/Device/ST/STM32F10x/startup_stm32f10x_md.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/system_stm32f10x.o : Syssup/CMSIS/Device/ST/STM32F10x/system_stm32f10x.c
	@$(CC) $(CFLAGS) -c $< -o $@

obj/misc.o : Syssup/STM32F10x_StdPeriph_Driver/src/misc.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/stm32f10x_adc.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_adc.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/stm32f10x_bkp.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_bkp.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/stm32f10x_can.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_can.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/stm32f10x_cec.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_cec.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/stm32f10x_crc.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_crc.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/stm32f10x_dac.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_dac.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/stm32f10x_dbgmcu.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_dbgmcu.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/stm32f10x_dma.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_dma.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/stm32f10x_exti.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_exti.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/stm32f10x_flash.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_flash.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/stm32f10x_fsmc.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_fsmc.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/stm32f10x_gpio.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/stm32f10x_i2c.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_i2c.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/stm32f10x_iwdg.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_iwdg.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/stm32f10x_pwr.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_pwr.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/stm32f10x_rcc.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/stm32f10x_rtc.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_rtc.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/stm32f10x_sdio.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_sdio.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/stm32f10x_spi.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_spi.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/stm32f10x_tim.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_tim.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/stm32f10x_usart.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_usart.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/stm32f10x_wwdg.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_wwdg.c
	@$(CC) $(CFLAGS) -c $< -o $@

# USB objects
obj/usb_core.o : Syssup/STM32_USB-FS-Device_Driver/src/usb_core.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/usb_init.o : Syssup/STM32_USB-FS-Device_Driver/src/usb_init.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/usb_int.o : Syssup/STM32_USB-FS-Device_Driver/src/usb_int.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/usb_mem.o : Syssup/STM32_USB-FS-Device_Driver/src/usb_mem.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/usb_regs.o : Syssup/STM32_USB-FS-Device_Driver/src/usb_regs.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/usb_sil.o : Syssup/STM32_USB-FS-Device_Driver/src/usb_sil.c
	@$(CC) $(CFLAGS) -c $< -o $@

obj/hw_config.o : USB/hw_config.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/stm32_it.o : USB/stm32_it.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/usb_desc.o : USB/usb_desc.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/usb_istr.o : USB/usb_istr.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/usb_prop.o : USB/usb_prop.c
	@$(CC) $(CFLAGS) -c $< -o $@
obj/usb_pwr.o : USB/usb_pwr.c
	@$(CC) $(CFLAGS) -c $< -o $@


firmware : $(ALLOBJS)
	@$(CC) $(TARGPROC) $(SHAREDC) -Wl,-Map=bin/firmware.map -Wl,--gc-sections -Wl,-Tlink.ld -o bin/firmware.elf $(ALLOBJS)
	@$(OBJCOPY) -O binary bin/firmware.elf bin/firmware.bin
	@$(OBJCOPY) -O ihex bin/firmware.elf bin/firmware.hex
	@$(SIZE) -B bin/firmware.elf

flash :
	openocd -f flash.cfg

startdebug :
	@$(GDB)
	openocd -f debug.cfg



clean :
	@echo Cleaning..
	@rm -f obj/*.o
	@rm -f bin/*.elf
	@rm -f bin/*.bin
	@rm -f bin/*.hex

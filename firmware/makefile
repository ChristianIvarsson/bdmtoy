
CC=arm-none-eabi-gcc
CPP=arm-none-eabi-c++
LD=arm-none-eabi-ld
AS=arm-none-eabi-as
OBJCOPY=arm-none-eabi-objcopy
SIZE=arm-none-eabi-size
# -mcpu=cortex-m3; ; -Wall; -ffunction-sections; -g; -O0; 

TARGPROC=-mcpu=cortex-m3 -mthumb

SHAREDC=-ffunction-sections -O1 -Wall

# Some of the library code is using if flags
CODEFLAGS=-DSTM32F103RB -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER

HEADERPATHS=\
	-ISyssup/CMSIS/Include \
	-ISyssup/CMSIS/Device/ST/STM32F10x \
	-ISyssup/STM32F10x_StdPeriph_Driver \
	-ISyssup/STM32F10x_StdPeriph_Driver/inc \
	-ISyssup/STM32_USB-FS-Device_Driver/inc \
	-IUSB

CFLAGS= $(TARGPROC) $(CODEFLAGS) $(HEADERPATHS) $(SHAREDC) -std=gnu99 
CPPFLAGS= $(TARGPROC) $(CODEFLAGS) $(HEADERPATHS) $(SHAREDC)

LDFLAGS= $(TARGPROC) -nostartfiles --gc-sections

.PHONY: all flash

all : build/startup.o firmware

flash : all


TAPOBJS=\
	build/TAP_shared.o \
	build/TAP_lowlev.o \
	build/BDMs_high.o \
	build/BDMs_low.o \
	build/BDMn_high.o \
	build/BDMn_low.o \
	build/BDMo_high.o \
	build/BDMo_low.o \
	build/JTAG_high.o \
	build/NEXUS_high.o \
	build/UARTMON_high.o


SYSOBJS=\
	build/system_stm32f10x.o \
	build/misc.o \
	build/stm32f10x_adc.o \
	build/stm32f10x_bkp.o \
	build/stm32f10x_can.o \
	build/stm32f10x_cec.o \
	build/stm32f10x_crc.o \
	build/stm32f10x_dac.o \
	build/stm32f10x_dbgmcu.o \
	build/stm32f10x_dma.o \
	build/stm32f10x_exti.o \
	build/stm32f10x_flash.o \
	build/stm32f10x_fsmc.o \
	build/stm32f10x_gpio.o \
	build/stm32f10x_i2c.o \
	build/stm32f10x_iwdg.o \
	build/stm32f10x_pwr.o \
	build/stm32f10x_rcc.o \
	build/stm32f10x_rtc.o \
	build/stm32f10x_sdio.o \
	build/stm32f10x_spi.o \
	build/stm32f10x_tim.o \
	build/stm32f10x_usart.o \
	build/stm32f10x_wwdg.o

USBOBJS=\
	build/usb_core.o \
	build/usb_init.o \
	build/usb_int.o \
	build/usb_mem.o \
	build/usb_regs.o \
	build/usb_sil.o \
	build/hw_config.o \
	build/stm32_it.o \
	build/usb_desc.o \
	build/usb_istr.o \
	build/usb_prop.o \
	build/usb_pwr.o






ALLOBJS=\
	$(SYSOBJS) \
	$(USBOBJS) \
	$(TAPOBJS) \
	build/main.o \
	build/SupportFuncs.o \
	build/syscalls.o

# Only included when poking around
#	build/printf.o

# Interface objects
build/TAP_shared.o : TAP/TAP_shared.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/TAP_lowlev.o : TAP/TAP_lowlev.s
	@$(AS) $(ASFLAGS) -c $< -o $@
build/BDMs_high.o : TAP/BDM_HC12/BDMs_high.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/BDMs_low.o : TAP/BDM_HC12/BDMs_low.s
	@$(AS) $(ASFLAGS) -c $< -o $@
build/BDMn_high.o : TAP/BDM_NEW/BDMn_high.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/BDMn_low.o : TAP/BDM_NEW/BDMn_low.s
	@$(AS) $(ASFLAGS) -c $< -o $@
build/BDMo_high.o : TAP/BDM_OLD/BDMo_high.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/BDMo_low.o : TAP/BDM_OLD/BDMo_low.s
	@$(AS) $(ASFLAGS) -c $< -o $@
build/JTAG_high.o : TAP/JTAG/JTAG_high.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/NEXUS_high.o : TAP/NEXUS/NEXUS_high.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/UARTMON_high.o : TAP/UART_MON/UARTMON_high.c
	@$(CC) $(CFLAGS) -c $< -o $@


# Misc / main code
build/main.o : main.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/SupportFuncs.o : SupportFuncs.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/printf.o : Syssup/stdio/printf.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/syscalls.o : Syssup/syscalls/syscalls.c
	@$(CC) $(CFLAGS) -c $< -o $@


# System objects
build/startup.o : Syssup\CMSIS\Device\ST\STM32F10x/startup_stm32f10x_md.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/system_stm32f10x.o : Syssup\CMSIS\Device\ST\STM32F10x/system_stm32f10x.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/misc.o : Syssup/STM32F10x_StdPeriph_Driver/src/misc.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/stm32f10x_adc.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_adc.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/stm32f10x_bkp.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_bkp.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/stm32f10x_can.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_can.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/stm32f10x_cec.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_cec.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/stm32f10x_crc.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_crc.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/stm32f10x_dac.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_dac.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/stm32f10x_dbgmcu.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_dbgmcu.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/stm32f10x_dma.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_dma.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/stm32f10x_exti.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_exti.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/stm32f10x_flash.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_flash.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/stm32f10x_fsmc.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_fsmc.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/stm32f10x_gpio.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/stm32f10x_i2c.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_i2c.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/stm32f10x_iwdg.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_iwdg.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/stm32f10x_pwr.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_pwr.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/stm32f10x_rcc.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/stm32f10x_rtc.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_rtc.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/stm32f10x_sdio.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_sdio.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/stm32f10x_spi.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_spi.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/stm32f10x_tim.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_tim.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/stm32f10x_usart.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_usart.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/stm32f10x_wwdg.o : Syssup/STM32F10x_StdPeriph_Driver/src/stm32f10x_wwdg.c
	@$(CC) $(CFLAGS) -c $< -o $@

# USB objects
build/usb_core.o : Syssup/STM32_USB-FS-Device_Driver/src/usb_core.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/usb_init.o : Syssup/STM32_USB-FS-Device_Driver/src/usb_init.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/usb_int.o : Syssup/STM32_USB-FS-Device_Driver/src/usb_int.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/usb_mem.o : Syssup/STM32_USB-FS-Device_Driver/src/usb_mem.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/usb_regs.o : Syssup/STM32_USB-FS-Device_Driver/src/usb_regs.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/usb_sil.o : Syssup/STM32_USB-FS-Device_Driver/src/usb_sil.c
	@$(CC) $(CFLAGS) -c $< -o $@


build/hw_config.o : USB/hw_config.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/stm32_it.o : USB/stm32_it.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/usb_desc.o : USB/usb_desc.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/usb_istr.o : USB/usb_istr.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/usb_prop.o : USB/usb_prop.c
	@$(CC) $(CFLAGS) -c $< -o $@
build/usb_pwr.o : USB/usb_pwr.c
	@$(CC) $(CFLAGS) -c $< -o $@








firmware : $(ALLOBJS)
	@$(CC) -ffunction-sections -Wl,--gc-sections -nostartfiles -Tlink.ld -o bin/firmware.elf  $(ALLOBJS)
	@$(OBJCOPY) -O binary bin/firmware.elf bin/firmware.bin
	@$(OBJCOPY) -O ihex bin/firmware.elf bin/firmware.hex
	@$(SIZE) -B bin/firmware.elf


flash :
	openocd -f flash.cfg

clean :
	@echo Cleaning..
	@rm -f build/*.o
	@rm -f bin/*.elf
	@rm -f bin/*.bin
	@rm -f bin/*.hex

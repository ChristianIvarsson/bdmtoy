.PHONY: clean all header

all:
	@echo " "
	@echo "Making CPU32 generic drivers"
	@echo " "
	@$(MAKE) --no-print-directory -C flash/driver_16
	@$(MAKE) --no-print-directory -C md5
	@$(MAKE) --no-print-directory -C cmfi

header: all
	@./headertool flash/driver_16/out/driver_16bit.bin cpu32_flash16.h CPU32_flashdriver16
	@./headertool md5/out/md5.bin cpu32_md5.h CPU32_md5
	@./headertool cmfi/out/cpu32mcfi.bin cpu32_cmfi.h CPU32_cmfi

clean:
	@$(MAKE) --no-print-directory -C flash/driver_16 clean
	@$(MAKE) --no-print-directory -C md5 clean
	@$(MAKE) --no-print-directory -C cmfi clean

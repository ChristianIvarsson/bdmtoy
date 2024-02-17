.PHONY: clean all header

all:
	@echo " "
	@echo "Making CPU32 generic drivers"
	@echo " "
	@$(MAKE) --no-print-directory -C flash/driver
	@$(MAKE) --no-print-directory -C md5

header: all
	@./headertool flash/driver/out/gendriver.bin cpu32_flash.h CPU32_flashdriver
	@./headertool md5/out/md5.bin cpu32_md5.h CPU32_md5

clean:
	@$(MAKE) --no-print-directory -C flash/driver clean
	@$(MAKE) --no-print-directory -C md5 clean

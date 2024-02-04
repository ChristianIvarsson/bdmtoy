.PHONY: clean all header

all:
	@echo " "
	@echo "Making CPU32 generic flash driver"
	@echo " "
	@$(MAKE) --no-print-directory -C driver
	@$(MAKE) --no-print-directory -C detect

header: all
	@echo " "
	@echo "Generating header files.."
	@./headertool driver/out/gendriver.bin driver/gendriver.h genDriver
	@./headertool detect/out/gendetect.bin detect/gendetect.h genDetect

clean:
	@$(MAKE) --no-print-directory -C driver clean
	@$(MAKE) --no-print-directory -C detect clean

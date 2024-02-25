.PHONY: clean all header

all:
	@echo " "
	@echo "Making CPU32 specific drivers"
	@echo " "
	@$(MAKE) --no-print-directory -C f375_md5


header: all
	@./headertool f375_md5/out/md5.bin cpu32_f375_md5.h CPU32_f375_md5

clean:
	@$(MAKE) --no-print-directory -C f375_md5 clean

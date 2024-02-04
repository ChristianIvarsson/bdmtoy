OUTPUT_FORMAT("elf32-m68k", "elf32-m68k", "elf32-m68k")
OUTPUT_ARCH(m68k)

MEMORY {
	SRAM (RWX) : ORIGIN = 0, LENGTH = 2048
}

SECTIONS {
	.text : {
		KEEP(*(.text.vectors))
		KEEP(*(.text.entry))
		*(.text) 
	} > SRAM	

	.data : {
		*(.data)
	} > SRAM

	.bss : {
		*(.bss)
	} > SRAM
}

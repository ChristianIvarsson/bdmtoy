OUTPUT_FORMAT("elf32-m68k", "elf32-m68k", "elf32-m68k")
OUTPUT_ARCH(m68k)

/*
Driver: 1000 bytes
Stack:    24 bytes
*/

MEMORY {
	SRAM (RWX) : ORIGIN = 0, LENGTH = 1000
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

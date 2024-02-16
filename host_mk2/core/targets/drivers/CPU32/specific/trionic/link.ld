OUTPUT_FORMAT("elf32-m68k", "elf32-m68k", "elf32-m68k")
OUTPUT_ARCH(m68k)

/*
0x100000 - 0x1003FF ( 1024 ) : Buffer    
0x100400 - 0x1007E7 ( 1000 ) : Driver    
0x1007E8 - 0x1007FF (   24 ) : Stack     
*/

MEMORY {
    SRAM (RWX) : ORIGIN = 0x100400, LENGTH = 1000
}

SECTIONS {
	.text : {
		KEEP(*(.text.vectors))
		KEEP(*(.text.entry))
		KEEP(*(.text))
	} > SRAM	

	.data : {
		*(.data)
	} > SRAM

	.bss : {
		*(.bss)
	} > SRAM
}

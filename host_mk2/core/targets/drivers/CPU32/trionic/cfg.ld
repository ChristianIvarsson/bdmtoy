OUTPUT_FORMAT("binary")
OUTPUT_ARCH(m68k)

MEMORY {
    ram : ORIGIN = 0x100000, LENGTH = 0x800
}

SECTIONS{
    .text 0x100400  : {  
        *(.text) 
    } > ram    

    .data : {
        *(.rodata)
        *(.data)
    } > ram

    .bss : {
        *(.bss)
    } > ram
}

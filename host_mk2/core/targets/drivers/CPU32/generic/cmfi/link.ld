OUTPUT_FORMAT("elf32-m68k", "elf32-m68k", "elf32-m68k")
OUTPUT_ARCH(m68k)
ENTRY(entry)

MEMORY {
    SRAM   (RWX) : ORIGIN = 0x100000, LENGTH = 1920
}

SECTIONS {

    .init : {
        KEEP(*(.init))
    } > SRAM

    .text : {
        *(.text)
    } > SRAM

    .data : {      *(.data)  } > SRAM
    .bss  : {      *(.bss)   } > SRAM
}

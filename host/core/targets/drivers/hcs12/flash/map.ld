OUTPUT_ARCH(m68hc12)

MEMORY {
	ram   : ORIGIN = 0x2000, LENGTH = 0x2000
}

/* _start = 0x2000; */

PROVIDE (__stack = 0x3FF0);

SECTIONS{

	.text  0x2000  : {
	
		__s_text = . ;	
		*     (.text) 
		CONSTRUCTORS
		__e_text = . ; 
	} > ram

	.rodata : {

		__s_rodata = . ;
		*(.rodata)
		__e_rodata = . ;
	} > ram

	.data : {

		__s_data = . ;
		*(.data)
		__e_data = . ;
	} > ram

	.bss : {
	
		__s_bss = . ;
		*(.bss)
		*(COMMON)
		__e_bss = . ;
	} > ram
}
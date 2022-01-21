OUTPUT_FORMAT("binary")
OUTPUT_ARCH(powerpc)

MEMORY { ram : ORIGIN = 0x3F8000, LENGTH = 0x4000 }

SECTIONS {
	.text  0x3F8000  : {
	
		__s_text = . ;	
		*     (.text) 
		CONSTRUCTORS
		__e_text = . ; 
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
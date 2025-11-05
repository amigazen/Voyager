
/*
** Module Link File for GNU ld
*/
SECTIONS
{
        /* Take all code from all objects and libraries. */

	.text : {
		Voyager_Start = . ;
		*(.rodata) *(.rosdata) *(.rdata) *(.init) *(.fini) *(.eini)
		*(.text)
		Voyager_End = . ;
	} > dram                                    

	/* Maybe I should /DISCARD/ the sections below, but it could
	   be more useful to let the final link stage decide about it */
	   
	.data  : { *(.data)  } > dram 
	.sdata : { *(.sdata) } > dram 
	.sbss  : { *(.sbss)  } > dram 
	.bss   : { *(.bss)   } > dram 
} 


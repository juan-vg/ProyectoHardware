SECTIONS
{
	. = 0x0C000000;
	.text : { *(.text) }
	_bdata = .;
	.data : { *(.data) }
	_edata = .;
	.rodata : { *(.rodata) } 
	_bbss = .;
	.bss : { *(.bss) }
	_ebss = .;	
}
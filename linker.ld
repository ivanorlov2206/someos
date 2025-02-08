ENTRY(_start)

SECTIONS
{
	. = 0x100000;
	.multiboot.data : {
		*(.multiboot.data)
	}

	.text : ALIGN(4K) {
		*(.text)
	}
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}
	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}
	
	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
		_stack_end = .;
		. += 16K;
		_stack_top = .;
	}
	_kernel_end = .;
}

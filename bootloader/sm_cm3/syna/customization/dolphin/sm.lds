MEMORY
{
	/* total 128KB */
	TCMRAM (rwx) : ORIGIN = 0x00000000, LENGTH = 124K
	QUEUERAM (rwx) : ORIGIN = 0x0001F000, LENGTH = 4K
}

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_asm_start)

initstack_size = 0x400;
stack_size = 0x4000;
heap_size = 0x6000;

SECTIONS
{
	.text : {
		__text_start = ABSOLUTE(.);
		*(.start)
		*(.text .text.*)
		*(.rodata .rodata.*)
		*(.got)
		. = ALIGN(8);
		__rt_init_descs_start__ = .;
		KEEP(*(rt_init_descs))
		__rt_init_descs_end__ = .;
		. = ALIGN(8);
		__rt_module_descs_start__ = .;
		KEEP(*(rt_module_descs))
		__rt_module_descs_end__ = .;
		. = ALIGN(8);
		__isr_descs_start__ = .;
		KEEP(*(isr_descs))
		__isr_descs_end__ = .;
		. = ALIGN(8);
		__gpio_isr_descs_start__ = .;
		KEEP(*(gpio_isr_descs))
		__gpio_isr_descs_end__ = .;
		. = ALIGN(4);
	} > TCMRAM

	.data : {
		*(.data .data.*)
		. = ALIGN(4);
	} > TCMRAM

	__text_end = ABSOLUTE(.);

	.bss : {
		__bss_start = ABSOLUTE(.);
		*(.bss .bss.*)
		. = ALIGN(4);
		__bss_end = ABSOLUTE(.);
	} > TCMRAM

	. = ALIGN(4);
	ucHeap = ABSOLUTE(.);
	__heap_start = ABSOLUTE(.);
	__heap_end = __heap_start + heap_size;

	. = ALIGN(4);
	__stack_end = __heap_end + 16;
	__stack_start = __stack_end + stack_size;

	__initstack_start = __stack_start + initstack_size;

	.msgqueue : {
		__queue_o1 = .;
		__queue_i0 = __queue_o1 + 0x200;
		__queue_i1 = __queue_o1 + 0x400;
		__queue_o0 = __queue_o1 + 0x600;
		__reserved = __queue_o1 + 0x800;
		__queue_end = __queue_o1 + 0x1000;
	} > QUEUERAM
}

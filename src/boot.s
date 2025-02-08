.set MAGIC, 0xE85250D6
.set ARCH, 0
.set LENGTH, 36
.set CHECKSUM, -(MAGIC+ARCH+LENGTH)

.section .multiboot.data, "aw"
.align 8
.long MAGIC
.long ARCH
.long LENGTH
.long CHECKSUM

.align 8
.short 5
.short 0
.long 20
.long 640
.long 480
.long 0

.section .text
.code32
.global _start
.extern bootinfo
.extern kernel_main
.type _start, @function
_start:
	lea bootinfo, %eax
	mov %ebx, (%eax)

	lea pdpte, %edi
	mov $0x2000, %ecx
	xor %eax, %eax
	rep stosb

	# Fill out pdpte
	mov $0, %ebx
	mov $pdpte, %edi
	or $0x3, %ebx
	or $128, %ebx
	mov %ebx, (%edi)

	# Fill out pml4e
	mov $pml4e, %edi
	mov $pdpte, %ebx
	or $0x3, %ebx
	mov %ebx, (%edi)

	mov $pml4e, %edi
	mov %edi, %cr3

	lidt idt_struct
	lgdt gdt_struct

	mov $0x20, %eax
	or $0x80, %eax
	mov %eax, %cr4

	mov $0xC0000080, %ecx
	rdmsr
	
	or $0x100, %eax
	wrmsr

	mov %cr0, %ebx
	or $0x80000001, %ebx
	mov %ebx, %cr0

	mov $16, %ax
	mov %ax, %ss
	mov %ax, %gs
	mov %ax, %fs
	mov %ax, %es
	mov %ax, %ds

	jmp $0x8,$long_mode

idt_struct:
.short 0
.word 0

gdt_segs:
	.quad 0
	.quad 0x00209A0000000000
	.quad 0x0000920000000000
.align 4
gdt_struct:
.short . - gdt_segs - 1
.quad gdt_segs

.code64
long_mode:
	lea _stack_top, %rsp
	lea _stack_top, %rbp
	call kernel_main


.section .data
pdpte:
.skip 4096

pml4e:
.skip 4096

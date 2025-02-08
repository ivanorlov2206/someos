#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include "desc.h"
#include "console.h"
#include "paging.h"
#include "printk.h"
#include "multiboot_stuff.h"
#include "alloc.h"
#include "pit.h"
#include "pic.h"
#include "acpi.h"

/*static void play_w_apic(void)
{
	uint32_t edx, eax;
	asm("mov $0x1b, %%ecx\n"
	    "rdmsr\n"
	    "mov %%edx, %0\n"
	    "mov %%eax, %1":"=r"(edx),"=r"(eax)::);

	eax |= (1 << 11);
	asm("wrmsr"::"a"(eax), "d"(edx), "c"(0x1b));

	uint64_t lapic_addr = ((eax >> 12) | ((edx & 0xFFFFF) << 20)) << 12;
	printk("LAPIC addr: %x\n", lapic_addr);
	map_addr(lapic_addr, lapic_addr);
	uint32_t apic_id = (*((uint32_t *)lapic_addr + 0x20)) >> 24;
	printk("LAPIC ID: %x\n", apic_id);
}*/

void kernel_main(void)
{
	uint32_t ecx, edx;

	global_console->init(global_console);
	phy_page_allocator->init();	

	setup_idt_entries();
	init_paging();

	read_multiboot_headers();

	init_kalloc();

	void *data = kalloc(32);
	void *data2 = kalloc(513);
	void *data3 = kalloc(8);
	void *data4 = kalloc(16);
	void *data5 = kalloc(1049);
	void *data6 = kalloc(4097);
	pr_info("Allocated data: %x\n", data);
	pr_info("Allocated data2: %x\n", data2);
	pr_info("Allocated data3: %x\n", data3);
	pr_info("Allocated data4: %x\n", data4);
	pr_info("Allocated data5: %x\n", data5);
	pr_info("Allocated data5: %x\n", data6);
	kfree(data);
	kfree(data4);
	kfree(data3);
	kfree(data5);
	data2 = kalloc(128);
	kfree(data6);
	pr_info("Allocated data: %x\n", data2);

	print_rsdp();
	while(1);
	//set_periodic_ticking();
	//init_pic();
	//asm("int $0x20");
	for (int i = 0; i < 128; i++) {
		pr_info("Testing row iteration: %u\n", i);
		for(uint64_t j = 0; j < 10000000; j++);
	}

	/*asm("mov $0x01, %%eax\n"
	    "cpuid\n"
	    "mov %%ecx, %0\n"
	    "mov %%edx, %1":"=r"(ecx), "=r"(edx)::"eax","ecx","edx");

	bool is_apic_supported = edx & (1 << 9);

	printk("%x\n", 0xFEE00000LU);

	if (is_apic_supported) {
		printk("APIC is supported\n");
		play_w_apic();
	}*/

	while(1);

	asm("hlt");
}

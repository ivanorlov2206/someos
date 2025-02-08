#include "acpi.h"
#include "string.h"
#include <stdbool.h>
#include "string.h"
#include "printk.h"
#include "paging.h"

struct RSDP rsdp;
bool inited;

void init_rsdp(struct multiboot_tag_old_acpi *acpi)
{
	inited = true;
	rsdp = *((struct RSDP *)acpi->rsdp);
}

void print_rsdp(void)
{
	char lbl[5];
	lbl[4] = 0;
	printk("%u RSDT Addr: %x\n", inited, rsdp.rsdt_addr);
	map_addr(rsdp.rsdt_addr, rsdp.rsdt_addr);

	struct RSDT *rsdt = (struct RSDT *)rsdp.rsdt_addr;
	uint64_t size = (rsdt->h.length - sizeof(rsdt->h)) / 4;
	pr_info("Listing ACPI info...\n");
	for (uint64_t i = 0; i < size; i++) {
		struct ACPISDTHeader *hdr = (struct ACPISDTHeader *)rsdt->ptr_to_sdts[i];
		memcpy(lbl, hdr->signature, 4);
		printk("%s\n", lbl);
	}
}

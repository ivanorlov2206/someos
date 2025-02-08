#ifndef _SOMELIB_ACPI
#define _SOMELIB_ACPI
#include "multiboot2.h"
#include <stdint.h>

struct RSDP {
	char signature[8];
	uint8_t checksum;
	char OEMID[6];
	uint8_t revision;
	uint32_t rsdt_addr;
} __attribute__((packed));

struct ACPISDTHeader {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char OEMID[6];
	char OEMTableID[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
} __attribute__((packed));

struct RSDT {
	struct ACPISDTHeader h;
	uint32_t ptr_to_sdts[];
} __attribute__((packed));

void init_rsdp(struct multiboot_tag_old_acpi *acpi);

#endif

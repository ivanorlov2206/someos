#include "multiboot_stuff.h"
#include "printk.h"
#include "framebuffer.h"
#include "paging.h"

#define MMAP_HDR_SIZE 16

void *bootinfo;

static void process_memory_map(struct multiboot_tag_mmap *mmap)
{
	uint64_t num_entries = (mmap->size - MMAP_HDR_SIZE) / mmap->entry_size;

	pr_info("Found memory map. Listing entries...\n");
	for (uint64_t i = 0; i < num_entries; i++) {
		pr_info("Addr: %x len: %x type: %x\n", mmap->entries[i].addr,
			mmap->entries[i].len, mmap->entries[i].type);
		if (mmap->entries[i].type != MULTIBOOT_MEMORY_AVAILABLE) {
			phy_page_allocator->mark_as_taken(mmap->entries[i].addr,
					mmap->entries[i].addr + mmap->entries[i].len);
		}
	}
}

#define CEIL8(x) ((x + 7) / 8 * 8)

void read_multiboot_headers(void)
{
	struct multiboot_tag *h;

	h = (struct multiboot_tag *)(bootinfo + 8);
	pr_info("Reading the multiboot information structure...\n");
	while(h->type != 0 || h->size != 8) {
		pr_warn("Found tag: %u, Size: %u\n", h->type, h->size);

		switch (h->type) {
		case MULTIBOOT_TAG_TYPE_MMAP:
			process_memory_map((struct multiboot_tag_mmap *)h);
			break;
		case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
			init_framebuffer((struct multiboot_tag_framebuffer *)h);
			break;
		case MULTIBOOT_TAG_TYPE_ACPI_OLD:
			init_rsdp((struct multiboot_tag_old_acpi *)h);
			break;
		}

		h = (struct multiboot_tag *)CEIL8((uint64_t)h + h->size);
	}
}

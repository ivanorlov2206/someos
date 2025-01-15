#include "string.h"
#include "paging.h"

static uint64_t *pml4;

void map_addr(uint64_t va, uint64_t pa)
{
	uint64_t i, ci;
	uint64_t *cur_p = pml4;

	va &= ~(PAGE_SIZE - 1);
	pa &= ~(PAGE_SIZE - 1);

	va >>= 12;
	for (i = 3; i > 0; i--) {
		ci = (va >> (9 * i)) & 0x1FF;
		if (!cur_p[ci]) {
			cur_p[ci] = (uint64_t)phy_page_allocator->get();
			memset((uint8_t *)cur_p[ci], 0, PAGE_SIZE);
			cur_p[ci] |= 3;
		}
		cur_p = (uint64_t *)(cur_p[ci] & ~(PAGE_SIZE - 1));
	}
	cur_p[va & 0x1FF] = pa | 0x3;
}

void map_range(uint64_t pstart, uint64_t pend, uint64_t vstart)
{
	pstart &= ~(PAGE_SIZE - 1);
	pend &= ~(PAGE_SIZE - 1);

	while (pstart != pend) {
		map_addr(pstart, vstart);
		pstart += PAGE_SIZE;
		vstart += PAGE_SIZE;
	}
}

void init_paging(void)
{
	pml4 = phy_page_allocator->get();
	memset((uint8_t *)pml4, 0, PAGE_SIZE);

	map_range(0, 1024 * 1024 * 1024, 0);

	asm("mov %0, %%rdi\n"
	    "mov %%rdi, %%cr3"::"r"(pml4));
}

#include <stdint.h>
#include <stddef.h>
#include "paging.h"
#include "bitmap_phy_alloc.h"

#define BITMAP_CELL_FULL 0xFFFFFFFFFFFFFFFFULL
#define BITMAP_CELL_SIZE (sizeof(uint64_t) * 8)
#define BITMAP_COUNT ((1024 * 1024) / BITMAP_CELL_SIZE) // First 4GB

static uint64_t pages_bitmap[BITMAP_COUNT];

static void *bitmap_phy_alloc_get_page(void)
{
	uint32_t i, j;

	for (i = 0; i < BITMAP_COUNT; i++) {
		if (pages_bitmap[i] == BITMAP_CELL_FULL)
			continue;

		for (j = 0; j < BITMAP_CELL_SIZE; j++)
			if (!(pages_bitmap[i] & (1LLU << j))) {
				pages_bitmap[i] |= (1LLU << j);
				return (void *)((i * 8 * sizeof(pages_bitmap[i]) + j) * PAGE_SIZE);
			}
	}
	
	return NULL;
}

static void bitmap_mark_as_taken(uint64_t start, uint64_t end)
{
	uint64_t ind, subind;

	start = start / PAGE_SIZE;
	end = (end + PAGE_SIZE - 1) / PAGE_SIZE;

	while (start != end) {
		ind = start / BITMAP_CELL_SIZE;
		subind = start % BITMAP_CELL_SIZE;

		pages_bitmap[ind] |= (1 << subind);

		start++;
	}

}

extern uint64_t _kernel_end;

static void bitmap_phy_alloc_init(void)
{
	bitmap_mark_as_taken(0, (uint64_t)&_kernel_end);
	bitmap_mark_as_taken(0xA0000, 0xFFFFF);
}

static void bitmap_phy_free(void *addr)
{
	uint64_t page_num = (uint64_t)addr / PAGE_SIZE;
	uint64_t ind = page_num / BITMAP_CELL_SIZE;
	uint64_t subind = page_num % BITMAP_CELL_SIZE;

	pages_bitmap[ind] &= ~(1LLU << subind);
}

struct page_allocator bitmap_phy_alloc = {
	.get = bitmap_phy_alloc_get_page,
	.free = bitmap_phy_free,
	.init = bitmap_phy_alloc_init,
	.mark_as_taken = bitmap_mark_as_taken,
};

#include <stddef.h>
#include <stdbool.h>

#include "alloc.h"
#include "paging.h"
#include "bitmap_phy_alloc.h"
#include "printk.h"

//#define DEBUG_KALLOC

#ifdef DEBUG_KALLOC
#define kalloc_debug(fmt, ...) pr_info(fmt, ##__VA_ARGS__)
#else
#define kalloc_debug(fmt, ...)
#endif

#define HEAP_SIZE (2 * 1024 * 1024)
#define HEAP_ADDR 0xFFFFFFFF00000000LLU
#define BINS_COUNT 128
#define MIN_BLOCK_POWER 6
#define MIN_BLOCK_SIZE (1LLU << MIN_BLOCK_POWER)

void *heap = (void *)HEAP_ADDR;
#define BLOCK_SIZE_MASK ~(1LLU)
#define BLOCK_TAKEN 1

#define BLOCK_SIZE(block) (((struct block_header *)block)->size & BLOCK_SIZE_MASK)

struct __attribute__((packed)) block_header {
	struct block_header *prev_in_bin;
	struct block_header *next_in_bin;
	uint64_t prev_size;
	uint64_t size;
};

struct block_header *bins[BINS_COUNT];

void init_kalloc(void)
{
	uint64_t i;
	uint64_t cur_va = (uint64_t)heap;

	pr_info("Initializing kalloc...\n");

	for (i = 0; i < HEAP_SIZE / PAGE_SIZE; i++) {
		uint64_t pa = (uint64_t)phy_page_allocator->get();
		if (!pa) {
			pr_fail("Failed to allocate heap");
			asm("hlt");
		}
		map_addr(cur_va, pa);

		cur_va += PAGE_SIZE;
	}

	struct block_header *main_block = (struct block_header *)heap;
	
	main_block->prev_in_bin = NULL;
	main_block->next_in_bin = NULL;
	main_block->prev_size = 0;
	main_block->size = HEAP_SIZE;

	bins[BINS_COUNT - 1] = main_block;
}

static uint64_t bin_ind(uint64_t size)
{
	if (!(size >> 9))
		return size >> 3;

	if (63 + (size >> 9) > BINS_COUNT - 1)
		return BINS_COUNT - 1;

	return 63 + (size >> 9);
}

static struct block_header *acquire_free_block(uint64_t size)
{
	uint64_t bin_i = bin_ind(size);

	kalloc_debug("Allocating block, size %u bin ind: %u\n", size, bin_i);
	while (bin_i < BINS_COUNT && (!bins[bin_i] || BLOCK_SIZE(bins[bin_i]) < size)) {
		bin_i++;
	};

	if (bin_i == BINS_COUNT)
		return NULL;

	struct block_header *min_block = bins[bin_i];
	struct block_header *cur = min_block;

	while (!cur) {
		if (BLOCK_SIZE(cur) < BLOCK_SIZE(min_block))
			min_block = cur;
		cur = cur->next_in_bin;
	}

	if (min_block->prev_in_bin)
		min_block->prev_in_bin->next_in_bin = min_block->next_in_bin;
	else
		bins[bin_i] = min_block->next_in_bin;

	if (min_block->next_in_bin)
		min_block->next_in_bin->prev_in_bin = min_block->prev_in_bin;

	return min_block;
}

static void return_block_to_bin(struct block_header *block)
{
	int64_t bin_i = bin_ind(block->size);

	kalloc_debug("Block with size %u returned to bin %u\n", block->size, bin_i);
	bug_on(bin_i < 0, "Block size is too small: %d %x", bin_i, BLOCK_SIZE(block));

	block->size &= ~BLOCK_TAKEN;
	block->prev_in_bin = NULL;
	block->next_in_bin = NULL;

	if (!bins[bin_i]) {
		bins[bin_i] = block;
		return;
	}

	struct block_header *cur_h = bins[bin_i];
	struct block_header *prev = NULL;

	while (cur_h) {
		if (cur_h->size >= block->size)
			break;

		prev = cur_h;
		cur_h = cur_h->next_in_bin;
	}

	prev->next_in_bin = block;
	block->prev_in_bin = prev;
	if (cur_h) {
		cur_h->prev_in_bin = block;
		block->next_in_bin = cur_h;
	}
}

void *kalloc(uint64_t size)
{
	struct block_header *new_block, *block;
	size = (size + 7) / 8 * 8;

	size += sizeof(struct block_header);
	block = acquire_free_block(size);

	bug_on(!block, "Can't acquire block");
	
	uint64_t rest_of_block = BLOCK_SIZE(block) - size;

	if (rest_of_block >= MIN_BLOCK_SIZE) {
		block->size = size;
		new_block = (struct block_header *)((uint64_t)block + size);
		new_block->size = rest_of_block;
		new_block->prev_size = size;
		return_block_to_bin(new_block);
	}
	
	block->size |= BLOCK_TAKEN;

	return (void *)((uint64_t)block + sizeof(struct block_header));
}

static inline bool block_exists(struct block_header *block)
{
	return (uint64_t)block < (uint64_t)heap + HEAP_SIZE;
}

static void extract_free_block(struct block_header *block)
{
	struct block_header *prev = block->prev_in_bin;

	if (!prev) {
		int64_t bin_i = bin_ind(block->size);
		bug_on(bins[bin_i] != block, "Block isn't in the right bin!");
		bins[bin_i] = block->next_in_bin;
	} else {
		prev->next_in_bin = block->next_in_bin;
	}
	if (block->next_in_bin)
		block->next_in_bin->prev_in_bin = prev;
}

static struct block_header *block_join_backwards(struct block_header *block)
{
	struct block_header *cur, *next;

	next = (struct block_header *)((uint64_t)block + BLOCK_SIZE(block));

	cur = (struct block_header *)((uint64_t)block - block->prev_size);
	while (block->prev_size && !(cur->size & BLOCK_TAKEN)) {
		kalloc_debug("Joining with the previous one\n");
		extract_free_block(cur);
		cur->size += BLOCK_SIZE(block);
		block = cur;
		cur = (struct block_header *)((uint64_t)block - block->prev_size);
	}

	if (block_exists(next))
		next->prev_size = BLOCK_SIZE(block);

	return block;
}

static void block_join_forward(struct block_header *block)
{
	struct block_header *cur;

	cur = (struct block_header *)((uint64_t)block + BLOCK_SIZE(block));
	while (block_exists(cur) && !(cur->size & BLOCK_TAKEN)) {
		kalloc_debug("Joining with the next one\n");
		block->size += BLOCK_SIZE(cur);
		extract_free_block(cur);
		cur = (struct block_header *)((uint64_t)cur + BLOCK_SIZE(cur));
	}

	if (block_exists(cur))
		cur->prev_size = BLOCK_SIZE(block);
}

void kfree(void *addr)
{
	struct block_header *block = (struct block_header *)(addr - sizeof(struct block_header));

	block = block_join_backwards(block);
	block_join_forward(block);

	return_block_to_bin(block);
}

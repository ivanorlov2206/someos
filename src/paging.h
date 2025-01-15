#ifndef _SOMELIB_PAGING
#define _SOMELIB_PAGING

#include <stdint.h>

#define PAGE_SIZE 4096
typedef void *(*palloc_getpage_t)();
typedef void (*palloc_init_t)();
typedef void (*palloc_free_t)(void *addr);
typedef void (*palloc_mark_as_taken_t)(uint64_t start, uint64_t end);

struct page_allocator {
	palloc_getpage_t get;
	palloc_free_t free;
	palloc_init_t init;
	palloc_mark_as_taken_t mark_as_taken;
};

void init_paging(void);
void map_addr(uint64_t va, uint64_t pa);
void map_range(uint64_t pstart, uint64_t pend, uint64_t vstart);

extern struct page_allocator *phy_page_allocator;

#endif

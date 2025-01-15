#ifndef _SOMELIB_ALLOC
#define _SOMELIB_ALLOC

#include <stdint.h>

void init_kalloc(void);
void *kalloc(uint64_t size);
void kfree(void *addr);

#endif

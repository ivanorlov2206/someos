#ifndef _SOMELIB_STRING
#define _SOMELIB_STRING
#include <stdint.h>

void memcpy(uint8_t *dst, uint8_t *src, uint64_t n);
void memset(uint8_t *dst, uint8_t c, uint64_t n);
#endif

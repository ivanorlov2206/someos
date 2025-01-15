#include <stdint.h>
#include "string.h"

void memcpy(uint8_t *dst, uint8_t *src, uint64_t n)
{
	for (uint64_t i = 0; i < n; i++) {
		*dst = *src;
		dst++;
		src++;
	}
}

void memset(uint8_t *dst, uint8_t c, uint64_t n)
{
	for (uint64_t i = 0; i < n; i++)
		*dst++ = c;
}

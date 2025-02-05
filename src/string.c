#include <stdint.h>
#include "string.h"

void memcpy(uint8_t *dst, uint8_t *src, uint64_t n)
{
	uint64_t *dst8 = (uint64_t *)dst;
	uint64_t *src8 = (uint64_t *)src;

	for (uint64_t i = 0; i < (n >> 3); i++)
		*dst8++ = *src8++;

	for (uint64_t i = 0; i < n % 8; i++)
		*dst++ = *src++;
}

void memset(uint8_t *dst, uint8_t c, uint64_t n)
{
	for (uint64_t i = 0; i < n; i++)
		*dst++ = c;
}

#ifndef _SOMELIB_PIO
#define _SOMELIB_PIO
#include <stdint.h>

static inline void outb(uint16_t port, uint8_t val)
{
	asm("outb %0, %1"::"a"(val), "r"(port));
}

static inline uint8_t inb(uint16_t port)
{
	uint8_t res;

	asm("inb %1, %0":"=a"(res):"Nd"(port));
	return res;
}

static inline void io_wait(void)
{
	outb(0x80, 0);
}

#endif

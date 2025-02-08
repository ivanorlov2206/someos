#include "pit.h"
#include <stdint.h>
#include "pio.h"

void set_periodic_ticking(void)
{
	uint8_t mode = 0b00110100;
	outb(0x43, mode);

	uint16_t freq = 0xFFFF;

	outb(0x40, freq & 0xFF);
	outb(0x40, (freq >> 8) & 0xFF);
}

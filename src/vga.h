#ifndef __SOMELIB_VGA
#define __SOMELIB_VGA

#include <stdint.h>
#include "console.h"

enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGNETA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGNETA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

typedef uint8_t vga_char_color_t;

static inline vga_char_color_t vga_char_color(enum vga_color bg, enum vga_color fg)
{
	return fg | (bg << 4);
}

struct vga_buffer {
	enum vga_color color;
	uint8_t columns;
	uint8_t rows;
	uint16_t *buffer_addr;
};

extern struct console vga_console;
#endif

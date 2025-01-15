#ifndef __SOMELIB_CONSOLE
#define __SOMELIB_CONSOLE
#include <stdint.h>

struct console;

enum console_color {
	CONSOLE_COLOR_DEFAULT,
	CONSOLE_COLOR_SUCCESS,
	CONSOLE_COLOR_FAIL,
	CONSOLE_COLOR_WARN,
};

typedef void (*console_init_t)(struct console *c);
typedef void (*console_putc_t)(struct console *c, char ch);
typedef void (*console_clear_t)(struct console *c);
typedef void (*console_set_color_t)(struct console *c, enum console_color color);

struct console {
	uint32_t col;
	uint32_t row;
	void *opaque;
	console_init_t init;
	console_putc_t putc;
	console_clear_t clear;
	console_set_color_t set_color;
};

extern struct console *global_console;

#endif

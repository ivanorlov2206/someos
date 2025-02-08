#include "printk.h"
#include "console.h"
#include <stdint.h>
#include <stdarg.h>

static char digit_char(uint64_t x)
{
	switch (x)
	{
	case 0 ... 9:
		return '0' + x;
	case 10 ... 15:
		return 'a' + (x - 10);
	}

	return '*';
}

static void print_num(uint64_t x, uint8_t base)
{
	char buf[32];
	uint32_t len = 0;

	if (!x) {
		global_console->putc(global_console, '0');
		return;
	}

	while (x) {
		buf[len++] = digit_char(x % base);
		x /= base;
	}

	while (len)
		global_console->putc(global_console, buf[--len]);
}

static void print_signed_num(int64_t x, uint8_t base)
{
	if (x < 0) {
		global_console->putc(global_console, '-');
		print_num(-x, base);
	} else {
		print_num((uint64_t)x, base);
	}
}

void printk(char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	char c;
	while (c = *fmt) {
		switch (c) {
		case '%':
			char modifier = *(++fmt);
			switch (modifier) {
			case 'x':
				uint64_t xu = va_arg(args, uint64_t);
				print_num(xu, 16);
				break;
			case 'u':
				uint64_t du = va_arg(args, uint64_t);
				print_num(du, 10);
				break;
			case 'd':
				int64_t ds = va_arg(args, int64_t);
				print_signed_num(ds, 10);
				break;
			case 's':
				char *s = va_arg(args, char *);
				while (*s)
					global_console->putc(global_console, *s++);
				break;
			case 'a':
				global_console->set_color(global_console, CONSOLE_COLOR_SUCCESS);
				break;
			case 'f':
				global_console->set_color(global_console, CONSOLE_COLOR_FAIL);
				break;
			case 'n':
				global_console->set_color(global_console, CONSOLE_COLOR_DEFAULT);
				break;
			case 'w':
				global_console->set_color(global_console, CONSOLE_COLOR_WARN);
				break;
			}
			break;
		default:
			global_console->putc(global_console, c);
		}

		fmt++;
	}
	va_end(args);
}

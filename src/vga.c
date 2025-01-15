#include <stdint.h>
#include "string.h"
#include "vga.h"
#include "console.h"

static uint16_t make_vga_entry(unsigned char c, vga_char_color_t color)
{
	return c | ((uint16_t)color << 8);
}

static void vga_put_char(struct vga_buffer *buffer,
		  unsigned char c, vga_char_color_t color,
		  uint8_t row, uint8_t col)
{
	if (row >= buffer->rows || col >= buffer->columns)
		return;

	uint16_t *entry_addr = buffer->buffer_addr + (row * buffer->columns) + col;
	*entry_addr = make_vga_entry(c, color);
}

static struct vga_buffer vbuffer = {
	.columns = 80,
	.rows = 25,
	.buffer_addr = (uint16_t *)0xB8000,
};

static vga_char_color_t vga_default_color;

static void vga_console_init(struct console *c)
{
	c->opaque = (void *)&vbuffer;

	vga_default_color = vga_char_color(VGA_COLOR_BLACK, VGA_COLOR_WHITE);
	vbuffer.color = vga_default_color;
}

static void vga_move_up(struct vga_buffer *buf, int n)
{
	int i, j;

	for (i = n; i < buf->rows; i++) {
		memcpy((uint8_t *)(buf->buffer_addr + (i - n) * buf->columns),
			(uint8_t *)(buf->buffer_addr + i * buf->columns),
			sizeof(uint16_t) * buf->columns);
		
	}

	for (i = buf->rows - n; i < buf->rows; i++) {
		for (j = 0; j < buf->columns; j++) {
			vga_put_char(buf, ' ', vga_default_color, i, j);
		}
	}
}

static void vga_console_putc(struct console *c, char ch)
{
	struct vga_buffer *buf = (struct vga_buffer *)c->opaque;

	if (ch == '\n') {
		c->row++;
		if (c->row == buf->rows) {
			vga_move_up(buf, 1);
			c->row--;
		}
		c->col = 0;
		return;
	}

	vga_put_char(buf, ch, buf->color, c->row, c->col++);
}

static void vga_console_clear(struct console *c)
{
	struct vga_buffer *buf = (struct vga_buffer *)c->opaque;

	for (int i = 0; i < buf->rows; i++) {
		for (int j = 0; j < buf->columns; j++) {
			vga_put_char(buf, ' ', vga_default_color, i, j);
		}
	}
}

static void vga_console_set_color(struct console *c, enum console_color color)
{
	struct vga_buffer *buf = (struct vga_buffer *)c->opaque;

	switch (color) {
	case CONSOLE_COLOR_SUCCESS:
		buf->color = vga_char_color(VGA_COLOR_BLACK, VGA_COLOR_GREEN);
		break;
	case CONSOLE_COLOR_WARN:
		buf->color = vga_char_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_BROWN);
		break;
	case CONSOLE_COLOR_FAIL:
		buf->color = vga_char_color(VGA_COLOR_BLACK, VGA_COLOR_RED);
		break;
	default:
		buf->color = vga_default_color;
		break;
	}
}

struct console vga_console = {
	.init = vga_console_init,
	.putc = vga_console_putc,
	.clear = vga_console_clear,
	.set_color = vga_console_set_color,
};

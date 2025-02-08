#include "pic.h"
#include "pio.h"
#include <stdint.h>

#define PIC1 0x20
#define PIC2 0xA0
#define PIC1_CMD (PIC1)
#define PIC1_DATA (PIC1 + 1)
#define PIC2_CMD (PIC2)
#define PIC2_DATA (PIC2 + 1)

#define ICW1_ICW4 (1 << 0)
#define ICW1_SINGLE (1 << 1)
#define ICW1_INTERVAL4 (1 << 2) // ??
#define ICW1_LEVEL (1 << 3)
#define ICW1_INIT (1 << 4)

#define ICW4_8086 (1 << 0)
#define ICW4_AUTO (1 << 1)
#define ICW4_BUF_SLAVE (1 << 3)
#define ICW4_BUF_MASTER 0xC
#define ICW4_SFNM 0x10

#define SLAVE_IRQ_2 4

#define OFFSET1 0x20
#define OFFSET2 0x28

void pic_set_mask(uint16_t mask)
{
	uint8_t val;

	outb(PIC1_DATA, mask & 0xFF);
	io_wait();
	outb(PIC2_DATA, (mask & 0xFF) >> 8);
	io_wait();
}

void init_pic()
{
	uint8_t m1, m2;

	m1 = inb(PIC1_DATA);
	m2 = inb(PIC2_DATA);
	asm("cli");

	outb(PIC1_CMD, ICW1_INIT | ICW1_ICW4);
	io_wait();
	outb(PIC2_CMD, ICW1_INIT | ICW1_ICW4);
	io_wait();
	outb(PIC1_DATA, OFFSET1);
	io_wait();
	outb(PIC2_DATA, OFFSET2);
	io_wait();
	outb(PIC1_DATA, SLAVE_IRQ_2);
	io_wait();
	outb(PIC2_DATA, ICW1_SINGLE);
	io_wait();

	outb(PIC1_DATA, ICW4_8086);
	io_wait();
	outb(PIC2_DATA, ICW4_8086);
	io_wait();

	outb(PIC1_DATA, 0xF0);
	outb(PIC2_DATA, 0xF0);
	asm("sti");
}

void send_eoi(void)
{
	outb(PIC2_CMD, 0x20);
	outb(PIC1_CMD, 0x20);
}

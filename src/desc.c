#include <stdint.h>
#include "desc.h"
#include "printk.h"
#include "pio.h"
#include "pic.h"

struct __attribute__((packed)) idt_entry {
	uint16_t offset0;
	uint16_t segment;
	uint8_t ist : 3;
	uint8_t _res0 : 5;
	uint8_t gate_type : 4;
	uint8_t _res1 : 1;
	uint8_t dpl : 2;
	uint8_t p : 1;
	uint16_t offset1;
	uint32_t offset2;
	uint32_t _res2;
};

struct __attribute__((packed)) desc {
	uint16_t size;
	uint64_t offset;
};

static struct idt_entry idt_entries[256] = {};
struct desc idt_desc = {
	.size = (uint16_t)(256 * sizeof(struct idt_entry) - 1),
	.offset = (uint64_t)&idt_entries,
};

static void idt_handler(uint8_t vector)
{
	printk("Interrupt has been triggered %u\n", vector);
	while(1);
}

#define SAVE_REGS asm("push %rsp\n"	\
	"push %rax\n"	\
	"push %rbx\n"			\
	"push %rcx\n"			\
	"push %rdx\n"			\
	"push %rsi\n"			\
	"push %rdi\n"			\
	"push %rbp\n"			\
	"push %r8\n"			\
	"push %r9\n"			\
	"push %r10\n"			\
	"push %r11\n"			\
	"push %r12\n"			\
	"push %r13\n"			\
	"push %r14\n"			\
	"push %r15\n"			\
);

#define RESTORE_REGS asm("pop %r15\n"	\
	"pop %r14\n"			\
	"pop %r13\n"			\
	"pop %r12\n"			\
	"pop %r11\n"			\
	"pop %r10\n"			\
	"pop %r9\n"			\
	"pop %r8\n"			\
	"pop %rbp\n"			\
	"pop %rdi\n"			\
	"pop %rsi\n"			\
	"pop %rdx\n"			\
	"pop %rcx\n"			\
	"pop %rbx\n"			\
	"pop %rax\n"			\
	"pop %rsp");

#define IRQ_HANDLER(num) \
static void __attribute__((naked)) irq_##num (void) {				\
	SAVE_REGS;								\
	idt_handler(num);							\
	RESTORE_REGS;								\
	asm("iretq");								\
}

static uint64_t counter = 0;

static void __attribute__((naked)) irq_32(void) {
	SAVE_REGS;
	printk("TIMER!! %u\n", counter++);
	send_eoi();
	RESTORE_REGS;
	asm("iretq");
}

IRQ_HANDLER(0); IRQ_HANDLER(1); IRQ_HANDLER(2); IRQ_HANDLER(3);
IRQ_HANDLER(4); IRQ_HANDLER(5); IRQ_HANDLER(6); IRQ_HANDLER(7);
IRQ_HANDLER(8); IRQ_HANDLER(9); IRQ_HANDLER(10); IRQ_HANDLER(11);
IRQ_HANDLER(12); IRQ_HANDLER(13); IRQ_HANDLER(14); IRQ_HANDLER(15);

IRQ_HANDLER(16); IRQ_HANDLER(17); IRQ_HANDLER(18); IRQ_HANDLER(19);
IRQ_HANDLER(20); IRQ_HANDLER(21); IRQ_HANDLER(22); IRQ_HANDLER(23);
IRQ_HANDLER(24); IRQ_HANDLER(25); IRQ_HANDLER(26); IRQ_HANDLER(27);
IRQ_HANDLER(28); IRQ_HANDLER(29); IRQ_HANDLER(30); IRQ_HANDLER(31);

/*IRQ_HANDLER(32);*/ IRQ_HANDLER(33); IRQ_HANDLER(34); IRQ_HANDLER(35);
IRQ_HANDLER(36); IRQ_HANDLER(37); IRQ_HANDLER(38); IRQ_HANDLER(39);
IRQ_HANDLER(40); IRQ_HANDLER(41); IRQ_HANDLER(42); IRQ_HANDLER(43);
IRQ_HANDLER(44); IRQ_HANDLER(45); IRQ_HANDLER(46); IRQ_HANDLER(47);

IRQ_HANDLER(48); IRQ_HANDLER(49); IRQ_HANDLER(50); IRQ_HANDLER(51);
IRQ_HANDLER(52); IRQ_HANDLER(53); IRQ_HANDLER(54); IRQ_HANDLER(55);
IRQ_HANDLER(56); IRQ_HANDLER(57); IRQ_HANDLER(58); IRQ_HANDLER(59);
IRQ_HANDLER(60); IRQ_HANDLER(61); IRQ_HANDLER(62); IRQ_HANDLER(63);

static void set_idt_entry(struct idt_entry *e, void *fn)
{
	uint64_t offset = (uint64_t)fn;

	e->offset0 = offset & 0xFFFF;
	e->offset1 = (offset >> 16) & 0xFFFF;
	e->offset2 = offset >> 32;
	e->segment = 0x8;
	e->_res1 = 0;
	e->p = 1;
	e->dpl = 0;
	e->gate_type = 0xE;
	e->ist = 0;
}

#define INIT_IDT_ENTRY(num) set_idt_entry(&idt_entries[num], irq_##num);

void setup_idt_entries(void)
{
	INIT_IDT_ENTRY(0); INIT_IDT_ENTRY(1); INIT_IDT_ENTRY(2); INIT_IDT_ENTRY(3);
	INIT_IDT_ENTRY(4); INIT_IDT_ENTRY(5); INIT_IDT_ENTRY(6); INIT_IDT_ENTRY(7);
	INIT_IDT_ENTRY(8); INIT_IDT_ENTRY(9); INIT_IDT_ENTRY(10); INIT_IDT_ENTRY(11);
	INIT_IDT_ENTRY(12); INIT_IDT_ENTRY(13); INIT_IDT_ENTRY(14); INIT_IDT_ENTRY(15);
	INIT_IDT_ENTRY(16); INIT_IDT_ENTRY(17); INIT_IDT_ENTRY(18); INIT_IDT_ENTRY(19);
	INIT_IDT_ENTRY(20); INIT_IDT_ENTRY(21); INIT_IDT_ENTRY(22); INIT_IDT_ENTRY(23);
	INIT_IDT_ENTRY(24); INIT_IDT_ENTRY(25); INIT_IDT_ENTRY(26); INIT_IDT_ENTRY(27);
	INIT_IDT_ENTRY(28); INIT_IDT_ENTRY(29); INIT_IDT_ENTRY(30); INIT_IDT_ENTRY(31);
	INIT_IDT_ENTRY(32); INIT_IDT_ENTRY(33); INIT_IDT_ENTRY(34); INIT_IDT_ENTRY(35);
	INIT_IDT_ENTRY(36); INIT_IDT_ENTRY(37); INIT_IDT_ENTRY(38); INIT_IDT_ENTRY(39);
	INIT_IDT_ENTRY(40); INIT_IDT_ENTRY(41); INIT_IDT_ENTRY(42); INIT_IDT_ENTRY(43);
	INIT_IDT_ENTRY(44); INIT_IDT_ENTRY(45); INIT_IDT_ENTRY(46); INIT_IDT_ENTRY(47);
	INIT_IDT_ENTRY(48); INIT_IDT_ENTRY(49); INIT_IDT_ENTRY(50); INIT_IDT_ENTRY(51);
	INIT_IDT_ENTRY(52); INIT_IDT_ENTRY(53); INIT_IDT_ENTRY(54); INIT_IDT_ENTRY(55);
	INIT_IDT_ENTRY(56); INIT_IDT_ENTRY(57); INIT_IDT_ENTRY(58); INIT_IDT_ENTRY(59);
	INIT_IDT_ENTRY(60); INIT_IDT_ENTRY(61); INIT_IDT_ENTRY(62); INIT_IDT_ENTRY(63);
	asm("lidt %0"::"m"(idt_desc));
}

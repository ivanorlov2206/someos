SRC := $(shell find src/ -name "*.c" | xargs -I {} sh -c 'basename {}')
OBJS := $(SRC:c=o)
all: boot $(OBJS)
	ld -Tlinker.ld -o someboot.elf boot.o $(OBJS)
	cp someboot.elf iso/boot/
	rm -rf os.iso
	grub-mkrescue -o os.iso iso
boot: src/boot.s
	gcc -c src/boot.s -o boot.o -nostdlib -ffreestanding -lgcc
%.o: src/%.c
	gcc -c $< -o $@ -nostdlib -ffreestanding -lgcc
run: all
	qemu-system-x86_64 -boot d --enable-kvm -d guest_errors,int,cpu_reset -cdrom os.iso -no-reboot -device VGA -serial stdio -hda disk.img
clean:
	rm -rf *.o *.elf
write: all
	sudo dd if=os.iso of=/dev/sda

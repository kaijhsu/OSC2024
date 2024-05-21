TARGET = main 
COPS = -nostdlib -mgeneral-regs-only

.PHONY: all dump clean

all:
	make clean
	aarch64-linux-gnu-gcc $(COPS) -c main.c -o main.o
	aarch64-linux-gnu-ld -T linker.ld main.o -static -o main.elf 
	aarch64-linux-gnu-objcopy main.elf -O binary main.img

dump: all
	qemu-system-aarch64 -M raspi3b -kernel main.img -display none -d in_asm

clean:
	rm -f main.o main.elf main.img main.d

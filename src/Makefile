all:
	# compiling the bootsector code
	nasm -f elf32 -g3 -F dwarf boot/32bit-main.asm -o build/bootsect.o
	ld -Ttext=0x7c00 -melf_i386 build/bootsect.o -o build/bootsect.elf
	objcopy -O binary build/bootsect.elf build/bootsect.img

	# compiling the kernel_entry code
	nasm -f elf32 -g3 -F dwarf boot/kernel_entry.asm -o build/kernel_entry.o

	# compiling the kernel
	i686-elf-gcc -m32 -ffreestanding -nostdlib -lgcc -g -c kernel/kernel.c -o build/kernel.o                
	i686-elf-ld -Ttext=0x1000 -melf_i386 build/kernel.o build/kernel_entry.o -o build/kernel.elf 
	objcopy -O binary build/kernel.elf build/kernel.img

	# build the os-image
	cat build/bootsect.img build/kernel.img > build/martinos.img

run:
	qemu-system-i386 -hda build/martinos.img

run_debug:
	qemu-system-i386 -hda build/martinos.img -s -S

debug_boot:
	gdb -x gdb_boot

debug_kernel:
	gdb -x gdb_kernel

clean:
	rm -rf build/*.elf build/*.img build/*.o 

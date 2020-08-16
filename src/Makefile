all:
	# compiling the bootsector code
	nasm -f elf32 -g3 -F dwarf boot/bootsector-main.asm -o build/bootsect.o
	ld -Ttext=0x7c00 -melf_i386 build/bootsect.o -o build/bootsect.elf
	objcopy -O binary build/bootsect.elf build/bootsect.img

	# padding file
	nasm -f bin boot/padding.asm -o build/padding.img

	# compiling the kernel_entry code
	nasm -f elf32 -g3 -F dwarf boot/kernel_entry.asm -o build/kernel_entry.o

	# compiling the idt.asm file
	nasm -f elf32 -g3 -F dwarf kernel/idt.asm -o build/idt.o

	# compiling the idt.asm file
	nasm -f elf32 -g3 -F dwarf kernel/paging.asm -o build/paging.o

	# compiling the kernel
	i686-elf-gcc -m32 -ffreestanding -nostdlib -lgcc -g -c -E kernel/kernel.c -o build/test_kernel.o    

	i686-elf-gcc -m32 -ffreestanding -nostdlib -lgcc -g -c kernel/kernel.c -o build/kernel.o    
	i686-elf-ld -Ttext=0x1000 -melf_i386 build/kernel.o build/kernel_entry.o build/idt.o build/paging.o -o build/kernel.elf 
	objcopy -O binary build/kernel.elf build/kernel.img

	# build the os-image
	cat build/bootsect.img build/kernel.img build/padding.img > build/martinos.img

run:
	qemu-system-i386 -hda build/martinos.img

run_debug_boot:
	qemu-system-i386 -hda build/martinos.img -s -S & 
	gdb -x gdb_boot

run_debug_kernel:
	qemu-system-i386 -hda build/martinos.img -s -S & 
	gdb -x gdb_kernel

debug_boot:
	gdb -x gdb_boot

debug_kernel:
	gdb -x gdb_kernel

clean:
	rm -rf build/*.elf build/*.img build/*.o 

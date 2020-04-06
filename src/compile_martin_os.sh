# compiling the bootsector code
nasm -f elf32 -g3 -F dwarf 32bit-main.asm -o build/bootsect.o
ld -Ttext=0x7c00 -melf_i386 build/bootsect.o -o build/bootsect.elf
objcopy -O binary build/bootsect.elf build/bootsect.img

# compile the kernel entry program
nasm -f elf32 -g3 -F dwarf kernel_entry.asm -o build/kernel_entry.o

# compiling the kernel
i686-elf-gcc -m32 -ffreestanding -nostdlib -lgcc -c kernel.c -o build/kernel.o

# link the two together: fill in the memory address of main
ld -Ttext=0x1000 --oformat binary -melf_i386 build/kernel_entry.o build/kernel.o -o build/kernel.img

# build the os-image
cat build/bootsect.img build/kernel.img > build/martinos.img
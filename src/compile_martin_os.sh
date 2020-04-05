# compiling the bootsector code
nasm -f elf -g3 -F dwarf 32bit-main.asm -o build/bootsect.o
ld -Ttext=0x7c00 -melf_i386 build/bootsect.o -o build/bootsect.elf
objcopy -O binary build/bootsect.elf build/bootsect.img

# compiling the kernel
gcc -ffreestanding kernel.c -o build/kernel.o
ld -o build/kernel.img -Ttext=0x1000 build/kernel.o

# build the os-image
cat build/bootsect.img build/kernel.img > build/martinos.img
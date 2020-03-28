So I also took a look at the Zesterner Bare Bones tutorial

https://wiki.osdev.org/User:Zesterer/Bare_Bones


in the guide it says 

        "The x86 is a complex architecture with various different CPU states and modes. 
        To avoid having to deal with them right now, we'll be using the GRUB bootloader to load our 
        kernel into memory and set up a stable 32-bit 'protected-mode' environment.

        To test our kernel, we'll be running it in QEMU. QEMU is an emulator that will allow us to 
        test our kernel without rebooting real hardware to test every change we make."


Itss important you know. Normally when you write C code in a hosted environment, you have a plethora or interfaces available to you. 
You can read from files, you can output messages, you can get user input... All with just a few lines of code. Sadly, we dontt have that. 
Those things are provided by an operating system. And right now, we dont have one since we ARE the operating system.

What we DO have access to however is a few useful headers GCC automatically provides us with (they give us things like fixed-width integers) 
and the hardware of the x86. We_d like to output text, so for that we r going to write ourselves a very simple driver that interacts with the x86_s 
VGA buffer and allows us to display text on the screen. In doing this, we will have to avoid using anything like the C standard library, because it simply isntt
available when we r compiling for a freestanding target like we are now.


so the way it compiles, the author uses the line 

                i686-elf-gcc -std=gnu99 -ffreestanding -g -c start.s -o start.o
                i686-elf-gcc -std=gnu99 -ffreestanding -g -c kernel.c -o kernel.o


-   ffreestanding tells the compiler to generate free-standing code (i.e: does not rely on an existing operating system to run).

the linking process, we uses 

                i686-elf-gcc -ffreestanding -nostdlib -g -T linker.ld start.o kernel.o -o mykernel.elf -lgcc


-   nostdlib is used to specify that we arent linking against a C standard library. This should be obvious, since we r running freestanding.


So apparently, you only need GRUB bootloader for the real hardware. QEMU has the ability to read Multiboot kernels build-in, so we dont need 
to go through the hassle of attaching our kernel to GRUB. To run your kernel with QEMU, you can just call your kernel image with qemu
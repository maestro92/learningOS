So in day 1, we built our GCC Cross-Compiler. 



I started with these few Bare_Bones tutorial: 
https://wiki.osdev.org/Bare_Bones
https://github.com/cfenollosa/os-tutorial/tree/master/01-bootsector-barebones


a couple of tools that will be used: 

-   The GNU Linker from Binutils to link your object files into the final kernel.
-   The GNU Assembler from Binutils (or optionally NASM) to assemble instructions into object files containing machine code.
-   The GNU Compiler Collection to compile your high level code into assembly.
-   The GRUB bootloader to bootload your kernel using the Multiboot boot protocol that loads us into 32-bit protected mode with paging disabled.
-   The ELF as the executable format that gives us control of where and how the kernel is loaded.


three input files 
boot.s - kernel entry point that sets up the processor environment
kernel.c - your actual kernel routines
linker.ld - for linking the above files










So lets move on to the next part of the tutorial where it talks about the kernel.c 

so the first line of the tutorial says 

        "The following shows how to create a simple kernel in C. This kernel uses the VGA text mode buffer (located at 0xB8000) as the output device."

VGA text mode apparenlty is just outputing 16 bit characters. The first 8 bit talks about the attributes of the character, such as color 
the 2nd 8 bit is the code point. 

the wikipedia article, has a pretty clear description under the "Data Arragement section".


so lets look at the code:

1.  first we do a bunch of #defines 


so a note about __i386__. 
https://en.wikipedia.org/wiki/Intel_80386

apparently i386 that is a 32-bit microprocessor from 1986. 
and __i386__ is a predefined macro. The c preprocessor (cpp), which is called by gcc, defines i386 (for i386 systems). 
Essentially in the logic of the preprocessor, when its compiling for the i386 systems, it defines this macro.

https://blog.kowalczyk.info/article/j/guide-to-predefined-macros-in-c-compilers-gcc-clang-msvc-etc..html




                #include <stdbool.h>
                #include <stddef.h>
                #include <stdint.h>
                 
                /* Check if the compiler thinks you are targeting the wrong operating system. */
                #if defined(__linux__)
                #error "You are not using a cross-compiler, you will most certainly run into trouble"
                #endif
                 
                /* This tutorial will only work for the 32-bit ix86 targets. */
                #if !defined(__i386__)
                #error "This tutorial needs to be compiled with a ix86-elf compiler"
                #endif
                 






2.  next we define a bunch of colors 

                enum vga_color {
                    VGA_COLOR_BLACK = 0,
                    VGA_COLOR_BLUE = 1,
                    VGA_COLOR_GREEN = 2,
                    VGA_COLOR_CYAN = 3,
                    VGA_COLOR_RED = 4,
                    VGA_COLOR_MAGENTA = 5,
                    VGA_COLOR_BROWN = 6,
                    VGA_COLOR_LIGHT_GREY = 7,
                    VGA_COLOR_DARK_GREY = 8,
                    VGA_COLOR_LIGHT_BLUE = 9,
                    VGA_COLOR_LIGHT_GREEN = 10,
                    VGA_COLOR_LIGHT_CYAN = 11,
                    VGA_COLOR_LIGHT_RED = 12,
                    VGA_COLOR_LIGHT_MAGENTA = 13,
                    VGA_COLOR_LIGHT_BROWN = 14,
                    VGA_COLOR_WHITE = 15,
                };                

these are just RGB. black is (0,0,0);. blue is (0,0,1); green is (0,1,0) ...
should be pretty straight forward. 



3.  lets first look at the kernel_main(void); function 


                void kernel_main(void) 
                {
                    /* Initialize terminal interface */
                    terminal_initialize();
                 
                    /* Newline support is left as an exercise. */
                    terminal_writestring("Hello, kernel World!\n");
                }



4.  lets look at the terminal_initialize(); function 

-   as you can see, we are first initalizing all of our vga_buffer as empty strings, ' '.

-   also note that we have the terminal_buffer pointed at (uint16_t*) 0xB8000; 
this is a hardcoded value. If you look at the wikipedia article (https://en.wikipedia.org/wiki/VGA-compatible_text_mode);
it says: 

                "The VGA text buffer is located at physical memory address 0xB8000. 
                Since this is usually used by 16-bit x86 processes operating in real-mode, 
                it is the first half of memory segment 0xB800. "


-   full code below:

                void terminal_initialize(void) 
                {
                    terminal_row = 0;
                    terminal_column = 0;
                    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
                    terminal_buffer = (uint16_t*) 0xB8000;
                    for (size_t y = 0; y < VGA_HEIGHT; y++) 
                    {
                        for (size_t x = 0; x < VGA_WIDTH; x++) 
                        {
                            const size_t index = y * VGA_WIDTH + x;
                            terminal_buffer[index] = vga_entry(' ', terminal_color);
                        }
                    }
                }



5.  now lets take a look at the terminal_writestring(); function 
                
-   essentially, we are managing two numbers here, terminal_row and terminal_column. That is our cursor position. 
as we writing each character into our terminal_buffer, we update our write cursor. 

                size_t terminal_row;
                size_t terminal_column;

                void terminal_writestring(const char* data) 
                {
                    terminal_write(data, strlen(data));
                }

                void terminal_write(const char* data, size_t size) 
                {
                    for (size_t i = 0; i < size; i++)
                        terminal_putchar(data[i]);
                }

                void terminal_putchar(char c) 
                {
                    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
                    if (++terminal_column == VGA_WIDTH) {
                        terminal_column = 0;
                        if (++terminal_row == VGA_HEIGHT)
                            terminal_row = 0;
                    }
                }
                 
                void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) 
                {
                    const size_t index = y * VGA_WIDTH + x;
                    terminal_buffer[index] = vga_entry(c, color);
                }



####################################################################################################
########################################## Linker Script ###########################################
####################################################################################################

in normal circumstances, if you dont use a linker script, the linker will use a default linker script that is compiled 
into the linker executable. However in kernel development, you will have to provde  your own customized linker script. 

the link below talks a lot about linker commands 
http://www.scoberlin.de/content/media/http/informatik/gcc_docs/ld_3.html

lets look at the linker script 


                /* The bootloader will look at this image and start execution at the symbol
                   designated as the entry point. */
                ENTRY(_start)
                 
                /* Tell where the various sections of the object files will be put in the final
                   kernel image. */
                SECTIONS
                {
                    /* Begin putting sections at 1 MiB, a conventional place for kernels to be
                       loaded at by the bootloader. */
                    . = 1M;
                 
                    /* First put the multiboot header, as it is required to be put very early
                       early in the image or the bootloader won't recognize the file format.
                       Next we'll put the .text section. */
                    .text BLOCK(4K) : ALIGN(4K)
                    {
                        *(.multiboot)
                        *(.text)
                    }
                 
                    /* Read-only data. */
                    .rodata BLOCK(4K) : ALIGN(4K)
                    {
                        *(.rodata)
                    }
                 
                    /* Read-write data (initialized) */
                    .data BLOCK(4K) : ALIGN(4K)
                    {
                        *(.data)
                    }
                 
                    /* Read-write data (uninitialized) and stack */
                    .bss BLOCK(4K) : ALIGN(4K)
                    {
                        *(COMMON)
                        *(.bss)
                    }
                 
                    /* The compiler may produce other sections, by default it will put them in
                       a segment with the same name. Simply add stuff here as needed. */
                }


-   ENTRY command
so the first think is the Entry point. The first instruction to execute in a program is the called the entry point. 
You use the ENTRY linker script command to set the entry point. The argument is a symbol name. 

                ENTRY(symbol)

The linker will set the entry point by tring each of the following methods in order, and stopping when one of them succeeds

                -   the '-e' entry command-line option;
                -   the ENTRY(symbol) command in a linker script;
                -   the value of the symbol start, if defined;
                -   the address of the first byte of the '.text' section, if present;
                -   The address 0.

so in our case, ENTRY(_start), we are setting _start as our entry point. Recall that _start is defined in our bootloader.c function;


-   SECTIONS command 
the SECTIONS command tells the linker how to map input sections into output sections, and how to place the output secions in memory. 

Each sections-command may of be one of the following:

                -   an ENTRY command (see section Setting the entry point)
                -   a symbol assignment (see section Assigning Values to Symbols)
                -   an output section description
                -   an overlay description

so we can only define output section that is supported by the format. For example a.out supports .text, .data and .bss.


###################################################################################
######################### Testing On QEMU #########################################
###################################################################################

So we will be using QEMU to test our Operating System 
according to wikipedia 

                QEMU is Quick EMUlator is a free and open-source emulator that performs hardware virtualization. 


                QEMU is a hosted virtual machine monitor: it emulates the machines processor through dynamic binary translation 
                and provides a set of different hardware and device models for the machine, enabling it to run a variety of guest operating systems. 
                It also can be used with KVM to run virtual machines at near-native speed (by taking advantage of hardware extensions such as Intel VT-x). 
                QEMU can also do emulation for user-level processes, allowing applications compiled for one architecture to run on another.


to install QEMU, i just followed this video.
https://www.youtube.com/watch?v=al1cnTjeayk

besure to add the executable to your windows Enviornment PATH.



in the "Testing your operating system (QEMU)", you can either run the myos.iso from the -cdrom option 

                qemu-system-i386 -cdrom myos.iso

or boot the multiboot kernels directly without bootable medium by doing 
                
                qemu-system-i386 -kernel myos.bin


I couldnt get GRUB to work in cygwin, so I just direclty ran myos.bin. And then it worked.

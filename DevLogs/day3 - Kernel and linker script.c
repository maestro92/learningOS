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
########################################## Linker Script  ##########################################
####################################################################################################
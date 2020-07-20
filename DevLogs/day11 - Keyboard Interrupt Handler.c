Lets look at how the keyboard handler works
so one thing we need to do is to enable external, maskable interrupts after we initalize the idt 
which we have to use the STI instruction 

so lets first do that 

                int kernel_main()
                {
                   test_print();

                   initIDT();
                   kprint("Init ID");

                   test_interrupt();

                   kprint("back here");

    ----------->   asm volatile("sti");
                }


now when you hit your keyboard, your irq1 handler should get called. 

################################################################
################### keyboard handler bugs ######################
################################################################

so one of the bugs I encountered is that my keyboard handler was only getting called once. 
Turns out that if you check this link out:
https://wiki.osdev.org/I_Can%27t_Get_Interrupts_Working

it says 

        When handling the keyboard IRQ, make sure that you read the byte the keyboard sends you. 
        The interrupt might not trigger again until it has been read:

                unsigned char scan_code = inb(0x60);

obviously in my code inb becomes port_byte_in(); and that fixed my keyboard handler problem. 


################################################################
######################### Scan Code History ####################
################################################################

So regarding scancodes, there are some useful links: 
https://www.win.tue.nl/~aeb/linux/kbd/scancodes-10.html#scancodesets

        The usual PC keyboards are capable of producing three sets of scancodes. 
        Writing 0xf0 followed by 1, 2 or 3 to port 0x60 will put the keyboard in scancode mode 1, 2 or 3. 
        Writing 0xf0 followed by 0 queries the mode, resulting in a scancode byte 43, 41 or 3f from the keyboard.

        Set 1 contains the values that the XT keyboard (with only one set of scancodes) produced, with extensions for new keys. 
        Someone decided that another numbering was more logical and invented scancode Set 2. 
        However, it was realized that new scancodes would break old programs, 
        so the keyboard output was fed to a 8042 microprocessor on the motherboard that could translate Set 2 back into Set 1. 
        Indeed a smart construction. This is the default today. Finally there is the PS/2 version, 
        Set 3, more regular, but used by almost nobody.


so the "XT keyboard" refers to the IBM Personal Computer XT 
https://en.wikipedia.org/wiki/IBM_Personal_Computer_XT
Essentially IBM produced these XT computers, and people generalized how the XT computers keyboards worked into scancode set 1. 
That is just how the IBM people designed it in 1983.  



-   regarding the Intel 8042 chip
Another thing about "8042 microprocessor". 8042 microprocessor is a specialized chip, specifically made 
to interface with the keyboard. The IBM PC AT used an Intel 8042 chip to interface to the keyboard. 

So what happens is that the 8042 microprocessor translates the incoming byte stream produced by the keyboard, 
and turns an f0 prefix into an OR with 80 for the next byte. 

Unless told not to translate, the keyboard controller translates keyboard scancodes into the 
scancodes it returns to the CPU using the following table mentioned in the link below.

note that all of this translation is done in the hardware. Like I said, its a specialized hardware that does the translation.

(Some implementations do this for the next byte that does not have this bit set already. 
A consequence is that in Set 3 the keys with Set-3 value 0x80 or more are broken in a peculiar way: 
hitting such a key and then some other key turns the make code for this last key into a break code. 
For example the Sleep key on a Microsoft Internet keyboard generates 54 / d4 for press/release. 
But pressing and releasing first Menu and then Sleep produces 8d 8d d4 d4 as translation of 8d f0 8d 54 f0 54. 
Other implementations are OK.)


Sets 2 and 3 are designed to be translated by the 8042. Set 1 should not be translated.
https://www.win.tue.nl/~aeb/linux/kbd/scancodes-10.html#:~:text=10.3%20Translation,have%20this%20bit%20set%20already.

so for our OS we will just support scancode set 1


################################################################
######################### Scan Code Mapping ####################
################################################################
So lets take a look at the scan code mapping 
http://www.quadibloc.com/comp/scan.htm

below you can see the scan code for set 1, set 2, set 3 and USB. As you can see the the list 
is listed in the order of set 1. 

I cant the design behind set 2 and set 3 on google yet. I guess that was just how the IBM people designed it 

so lets just focus on set 1. 

        01       ---->       Esc 
        02       ---->       ! 1
        03       ---->       @ 2

                 ....
                 ....

        10       ---->       Q 
        11       ---->       W


so conveniently we can just do a mapping table from scan code to ascii code 


esc maps to an ascii code of 27

                unsigned char keyboard_map[128] = 
                {
                  0,  27, /* Esc */
                  '1', '2', '3', '4', '5', '6', '7', '8', 
                  '9', '0', '-', '=', 
                  '\b', /* Backspace */
                  '\t', /* Tab */
                  'q', 'w', 'e', 'r',   
                  't', 'y', 'u', 'i', 'o', 'p', '[', ']', 
                  '\n', /* Enter key */
                    0,  /* 29   - Control */
                  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 39 */
                 '\'', '`',   
                    0,        /* Left shift */
                 '\\', 'z', 'x', 'c', 'v', 'b', 'n',            /* 49 */
                  'm', ',', '.', '/',   0,              /* Right shift */
                  '*',
                    0,  /* Alt */
                  ' ',  /* Space bar */
                    0,  /* Caps lock */
                    0,  /* 59 - F1 key ... > */
                    0,   0,   0,   0,   0,   0,   0,   0,
                    0,  /* < ... F10 */
                    0,  /* 69 - Num lock*/
                    0,  /* Scroll Lock */
                    0,  /* Home key */
                    0,  /* Up Arrow */
                    0,  /* Page Up */
                  '-',
                    0,  /* Left Arrow */
                    0,
                    0,  /* Right Arrow */
                  '+',
                    0,  /* 79 - End key*/
                    0,  /* Down Arrow */
                    0,  /* Page Down */
                    0,  /* Insert Key */
                    0,  /* Delete Key */
                    0,   0,   0,
                    0,  /* F11 Key */
                    0,  /* F12 Key */
                    0,  /* All other keys are undefined */
                };


then in our keyboard_handler(); function we have 



So for some reason that I cant explain 

                idt.c

                void irq1_handler()
                {
                    keyboard_handler();
                    port_byte_out(0x20, 0x20);
                }

                void keyboard_handler()
                {
                    unsigned char scan_code = port_byte_in(0x60);
                    unsigned char ascii = keyboard_map[scan_code];
                    char str[2] = {ascii, '\0'};
                    kprint(str);
                }



####################################################################
############################## Bugs ################################
####################################################################

for some reason I cant get

                unsigned char keyboard_map[128] = 
                {
                  0,  27, /* Esc */
                  '1', '2', '3', '4', '5', '6', '7', '8', 
                  '9', '0', '-', '=', 
                  '\b', /* Backspace */
                  '\t', /* Tab */
                  'q', 'w', 'e', 'r',   

                  ...
                  ...
                }

to work. When I look at GDB, the memory of keyboard_map is all zeros.
so the work around is 



                unsigned char keyboard_map[128];

                void init_keyboard_driver()
                {

                    unsigned char keyboard_map[128] = 
                    {
                      0,  27, /* Esc */
                      '1', '2', '3', '4', '5', '6', '7', '8', 
                      '9', '0', '-', '=', 
                      '\b', /* Backspace */
                      '\t', /* Tab */
                      'q', 'w', 'e', 'r',   

                      ...
                      ...
                    }

                    memory_copy(keyboard_map_temp, keyboard_map, 128);
                }

when I look at the martinOS.img, which is the binary I build, i do see the char array data for the keyboard_map

                ffff 61cf 60fc e836 feff ff61 cf8b 4424
                040f 0118 c3cd 02c3 001b 3132 3334 3536 <---------
                3738 3930 2d3d 0809 7177 6572 7479 7569
                6f70 5b5d 0a00 6173 6466 6768 6a6b 6c3b
                2760 005c 7a78 6376 626e 6d2c 2e2f 002a
                0020 0000 0000 0000 0000 0000 0000 0000
                0000 2d00 0000 2b00 0000 0000 0000 0000
                0000 0000 0000 0000 0000 0000 0000 0000
                0000 0000 0000 0000 0000 0000 0000 0000


but when I examine the memory in GDB, I cant get it to work. So I will just do this for now.
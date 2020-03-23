
####################################################################################
###################### 32 bit protected mode Print routine #########################
####################################################################################

continuing on the tutorial 
https://github.com/cfenollosa/os-tutorial/tree/master/08-32bit-print

32-bit protected mode 

32-bit mode allows us to use 32 bit registers and memory addressing, protected memory, virtual memory and other 
advantages, but we will lose BIOS interrupts and we will need to code the GDT, global descriptor table

so now we will write a new print function that works in protected mode. which doesnt work with BIOS interrupts,
but by directly manipulating the VGA video memory instead of calling the int 0x10.
The VGA memory starts at address 0xb8000 and it has a text mode which is useful to avoid manipulating direct pixels. 


real mode provides no support for memory protection, multitasking, or code privilege levels 


-   BIOS in protected mode 
So once you are in protected mode, almost all BIOS function become unavailable, and trying to call them will result in execeptions 
or unreliable response. 

The reason why the x86/x64 computers cant use BIOS interrupts when running protected mode is becuz 

1.  when running in protected mode, its no longer in 16-bit mode, so its not ready to correctly interpret the machine 
    code of the BIOS. 

2.  protected mode OS can change the memory layout according to its whims, so the BIOS can no longer be able to make 
    any assumptions about what address ranges it may safely use or where memory-mapped I/O is. For example, 
    the different way segment values are handled 

https://wiki.osdev.org/BIOS
https://www.reddit.com/r/askscience/comments/1ypv4t/why_can_x86x64_computers_not_use_bios_interrupts/



so to accomplish this, we need a few things 


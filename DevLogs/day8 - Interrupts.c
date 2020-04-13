so for some reason, the 
https://www.cs.bham.ac.uk/~exr/lectures/opsys/10_11/lectures/os-dev.pdf
is discontinued after writing the screen driver, so we have to opt to follow another tutorial.
We will now follow https://github.com/cfenollosa/os-tutorial/tree/master/18-interrupts
lesson 18. So we will learn how to do interrupts. 

the github link says its inspired by JamesM tutorial
https://web.archive.org/web/20160327011227/http://www.jamesmolloy.co.uk/tutorial_html/4.-The%20GDT%20and%20IDT.html

so lets look at what is the IDT, the Interrupt Descriptor Table


###########################################################
####################### Interrupts ########################
###########################################################

First we obviously need to understand what are interrupts 

an interrupt is a signal from a device, such as the keyboard, to the CPU, telling it to immediately stop whatever it is currenlty doing 
and do something else 

so imagine, your keyboard is connect to your motherboard, and it tells the CPU that it needs some attention when a key is pressed.

                                     _______________
                                    |               |
     _______________                |               |
    |               |               |               |
    |   keyboard    |---------------|   CPU         | 
    |_______________|               |               |
                                    |               |
                                    |_______________|


To know how a specific interrupt arise, the CPU has a table called the IDT (interrupt Descriptor Table).
This table is a vector table setup by the OS, and stored in memory. There are 256 interrupt vectors on x86 CPUs.

The number of interrupt vectors supported by a CPU differs based on the CPU architecture. 
There are generally 3 classes of interrupts on most platforms:

-   Exception
These are generated interally by the CPU and used to alert the running kernel of an event or situation which requires its attention.
On x86 CPUs, these execption includes such as Double Fault, Page Fault, General Protection Fault, etc.


-   Interrupt Request (IRQ) or Hardware Interrupt
      
hardware interrupts are used to handle events such as receiving data from a modem or network card, key presses or mouse movements  
https://en.wikipedia.org/wiki/Interrupt_request_(PC_architecture)




-   Software Interrupt 
This is an interrupt signalled by software to indicate that it needs the kernel_s attention. These types of interrupts are generally used for 
System Calls. On x86 CPUs, the instruction which is used to initiate a software interrupt is the "INT" instruction. 



https://wiki.osdev.org/Interrupts




-   Here lets go into more detail of an example

When a key is pressed, the keyboard controller tells the PIC to cause an interrupt. 

Becuz of the the wiring of keyboard and PIC, IRQ #1 is the keyboard interrupt, so when a key is pressed, IRQ 1 is sent to the PIC 

The role of the PIC will be to decide whether the CPU should be immediately notified of that IRQ or not and to translate 
the IRQ number into an interrupt vector (i.e. a number between 0 and 255) for the CPU_s table.

                                     _______________             _______________
                                    |               |           |               |
     _______________                |               |           |               |
    |               |               |  Programmable |           |               |
    |   keyboard    |---------------|  Interrupt    |-----------|    CPU        |
    |_______________|               |   Controller  |           |               |
                                    |               |           |               |
                                    |_______________|           |_______________|







-   Standard IRQ priority
https://wiki.osdev.org/Interrupts





-   From the CPU_s perspective 

Everytime the CPU is done with one machine instruction, it will check if the PIC_s pin has notified an interrupt. If that_s the case, it stores 
some state information on the stack (so that i can return to wahtever it is doing currently, when the INT is done being serviced by the OS)
and jumps to a location pointed to by the IDT. The OS takes over from there. 



#########################################################################
###################### Interrupt Descriptor Table #######################
#########################################################################






Theory:

so the Interrupt Descriptor Table is a data structure used by the x86 architecture to implement an interrupt vector table. 
its called Interrupt Vector Table (IVT) in real mode, and called Interrupt Descriptor Table (IDT) in protected mode.


https://en.wikipedia.org/wiki/Interrupt_descriptor_table
https://wiki.osdev.org/Interrupt_Descriptor_Table

Use of the IDT is triggered by three types of events: hardware interrupts, software interrupts, and processor exceptions, which 
together are referred to as interrupts.



-   Real mode
in the 8086 processor, the interrupt table is called IVT (interrupt vector table).
The IVT always residies at the same location in memory: the first 1024 bytes of memory at address 0x0000 to 0x03ff.
It consists of 256 four-byte real mode far pointers (256 * 4 = 1024 bytes of memory);

Each vector is 4 bytes long and contains the starting address of the interrupt handler. 
https://www.sciencedirect.com/topics/engineering/interrupt-vector


visually this is where IVT resdies in memory
                 ___________________    
                |                   |
                |                   |
                |                   |
                |                   |   
                |                   |
                |                   |
                |___________________|   <--------- 0x000BC000 (752 kb)
                |                   |
                |   CGA Graphics    |
                |   CGA, EGA &      |
                |   VGA Text        |
                |___________________|   <--------- 0x000B8000 (736 kb)
                |                   |
                |                   |   
                |                   |
                |                   |               
                |                   |
                |                   |
                |                   |
                |                   |
                |                   |
                |                   |
                |                   |
                |___________________|   <--------- 0x0003FF (1023 bytes)
                |                   |
                |                   |               
                |                   |
                |                   |
                |                   |
                |                   |
                |                   |
                |                   |
                |___________________|   <--------- 0x000000  

a real mode pointer is defined as a 16-bit segment and a 16-bit offset into that segment. 


The first 32 entries in the table is reserved for intel, as mandated by Intel. 

The special, CPU-dedicated interrupts are in the first 32 interrupts:

        0 - Division by zero exception
        1 - Debug exception
        2 - Non maskable interrupt
        3 - Breakpoint exception
        4 - 'Into detected overflow'
        5 - Out of bounds exception
        6 - Invalid opcode exception
        7 - No coprocessor exception
        8 - Double fault (pushes an error code)
        9 - Coprocessor segment overrun
        10 - Bad TSS (pushes an error code)
        11 - Segment not present (pushes an error code)
        12 - Stack fault (pushes an error code)
        13 - General protection fault (pushes an error code)
        14 - Page fault (pushes an error code)
        15 - Unknown interrupt exception
        16 - Coprocessor fault
        17 - Alignment check exception
        18 - Machine check exception
        19-31 - Reserved








-   Protected mode
The Interrupt Descriptor Table (IDT) is specific to the IA-32 architecture. As mentioned, IVT is called in the real mode,
which tells the location of the interrupt handlers. The IDT entries are called gates. It can contain Interrupt Gates, Task Gates, and Trap Gates.

The location of IDT (address and size) is kept in the IDTR register of the CPU, which can be loaded/stored using LIDT, SIDT instructions. 



the table contains 8-byte Gates entries. Each entry has a complex structure.


                struct IDTDescr {
                   uint16_t offset_1; // offset bits 0..15
                   uint16_t selector; // a code segment selector in GDT or LDT
                   uint8_t zero;      // unused, set to 0
                   uint8_t type_attr; // type and attributes, see below
                   uint16_t offset_2; // offset bits 16..31
                };





-   Gate Types 



https://www.scs.stanford.edu/05au-cs240c/lab/i386/s09_06.htm



https://wiki.osdev.org/Interrupt_Descriptor_Table



https://web.archive.org/web/20160327011227/http://www.jamesmolloy.co.uk/tutorial_html/4.-The%20GDT%20and%20IDT.html

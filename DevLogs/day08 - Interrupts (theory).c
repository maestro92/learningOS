so for some reason, the tutorial from 
(https://www.cs.bham.ac.uk/~exr/lectures/opsys/10_11/lectures/os-dev.pdf)
is discontinued after writing the screen driver, so we have to opt to follow another tutorial.

We will now follow (https://github.com/cfenollosa/os-tutorial/tree/master/18-interrupts)
lesson 18. So we will learn how to do interrupts. 


###########################################################
####################### Interrupts ########################
###########################################################

First we obviously need to understand what are interrupts 

an interrupt is a signal from a device, such as the keyboard, to the CPU, telling it to immediately stop whatever it is currenlty doing 
and do something else 

The thing you have to undestand is that x86 is a interrupt driven architecture. That is just how the intel guys designed it. 
and since that is how Intel says so, us as programmers need to setup interrupts properly for our OS. 


so there are two types of interrupts, hardware interrupts and software interrupts (soft irq);


External events trigger an interrupt. These events can be trigged by hardware or software. 



so imagine, your keyboard is connected to your motherboard, and it tells the CPU that it needs some attention when a key is pressed.

                                     _______________
                                    |               |
     _______________                |               |
    |               |               |               |
    |   keyboard    |-------------->|   CPU         | 
    |_______________|               |               |
                                    |               |
                                    |_______________|


To know how a specific interrupt is handled, the CPU has a table called the IDT (interrupt Descriptor Table).
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





-   Keyboard, PIC and CPU

When a key is pressed, the keyboard controller tells the PIC to cause an interrupt. 

Becuz of the the wiring of keyboard and PIC, IRQ #1 is the keyboard interrupt, so when a key is pressed, IRQ 1 is sent to the PIC 

The role of the PIC will be to decide whether the CPU should be immediately notified of that IRQ or not and to translate 
the IRQ number into an interrupt vector (i.e. a number between 0 and 255) for the CPU_s table.

                                     _______________             _______________
                                    |               |           |               |
     _______________                |               |           |               |
    |               |               |  Programmable |           |               |
    |   keyboard    |---------------|  Interrupt    |-----------|    CPU        |
    |_______________|               |  Controller   |           |               |
                                    |   (PIC)       |           |               |
                                    |_______________|           |_______________|




-   PIC
So to give an idea of how a PIC works, here is an example image:
https://hsto.org/webt/xj/yn/dh/xjyndhabujjwz0alrbx831zfzt8.png
https://habr.com/en/post/446312/

this is the intel 8259 PIC. This actually came out in 1976. Apparently, this 8259 PIC is one of the most important chips making up the x86 architecture.
without, the x86 architecture would not be an interrupt driven architecture. 
Do note that APIC has replaced 8259 PIC in more modern systems, especially those with multiple cores/processors. 

Obviously nowadays there are more advanced PICs. 
You can see that devices are connected to the PIC and then the PIC connects to the CPU through the INTR wire.
https://wiki.osdev.org/PIC

The 8259 PIC controls the CPU_s interrupt mechnaism. for example, when a keybaord registers a keyhit, it sends a pulse along its interrupt line (IRQ1)
to the PIC chip, wh ich then translates the IRQ into a system interrupt, and sends a message to interrupt the CPU from whatever it is doing. Part 
of the kernel_s job is to either handle these IRQs and perform the necessary procedures (poll the keyboard for the scancode) or alert
a userspace program to the itnerrupt (send a message ot the keyboard driver)

Without a PIC, you would have to poll all the devices in the system to see if they want to do anything (signal an event), but with a PIC, your 
system can run along incely until such time that a device wants to signal an event, which means you dont waste time going to the devices, 
you let the devices come to you when they are ready.

regarding which device connects to which IRQ line, that is actually all predetermined. The link Below shows a list of the Standard IRQ priority
https://en.wikipedia.org/wiki/Interrupt_request_(PC_architecture)


Just to show some example:

                            
                             ___________________________
                            |                           |
                            |                     IRQ 0 |-------- Programmable Interrupt Timer interrupt
                            |                           |
                            |                     IRQ 2 |-------- Keyboard
                            |                           |
                            |                     IRQ 3 |-------- Cascade
                            |                           |
                            |                     IRQ 4 |-------- COM2
                            |                           |
                            |                     IRQ 4 |-------- COM1
                            |                           |
                            |                     IRQ 5 |-------- parallel port 2 and 3 or sound card
                            |                           |
                            |                     IRQ 6 |-------- floppy disk controller
                            |                           |
                            |                     ...   |
                            |                     ...   |


There are actually two PICs on most systems, and each has 8 different inputs, plus one output signal that is used to tell the CPU that an IRQ occured.
Usually it contains a slave PIC and a master PIC. This is also shown in the link above: https://habr.com/en/post/446312/
THe slave PIC_s output signal is connected to the master PIC_s 3rd input (input #2). So when the slave PIC wants to tell the CPU an interrupt occured,
it actuallyt ells the master PIC, and the master PIC tells the CPU. This is called "Cascade". 

The master PIC_s 3r dinput is configured for this and not configured as a normal IRQ, which means that IRQ 2 cant happen. 

So if you notice, the interrupts are then handled by the priority level:
0, 1, 2, 8, 9, 10, 11, 12, 13, 14, 15, 3, 4, 5, 6, 7


When the PIC tells the CPU an interrupt occured, the CPU acknowledges the "interrupt occured" signal. Then the PIC chip sends the interrupt number 
(between 00h and FFH, or 0 and 255 in decimal) to the CPU. 



-   CPU 

Everytime the CPU is done with one machine instruction, it will check if the PIC_s pin has notified an interrupt. If that_s the case, it stores 
some state information on the stack (so that i can return to wahtever it is doing currently, when the INT is done being serviced by the OS)
and jumps to a location pointed to by the IDT. The OS takes over from there. 

The current program can prevent the CPU from being disturbed by interrupts by disabling the interrupt flag (IF in status register)
as long as this flag is cleared, the CPU ignores the PIC_s request and continues running the current program. You can control the flag 
through the CLI and STI flag. 


-   OS
OS will setup the IDT before hand. When the interrupt comes, we jumpt to the handler stored in the IDT to handle the interrupt. 
Usually the code interacts with the device, then returns to whatever it was doing previously with an iret instruction.
The PIC doesnt send any more interrupts until the CPU acknowledges the interrupt. 

In the example of a keyboard press, the interrupt handler asks the keyboard which key was pressed, does something with the information, then acknowledges 
and return. 


#########################################################################
###################### Interrupt Descriptor Table #######################
#########################################################################

So now you see that the OS will have to prepare the IDT, which contains all the interrupt handlers. 

The intel official spec also has a chapter on IDT
https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.html
Chapter 6.10





Theory:

so the Interrupt Descriptor Table is a data structure used by the x86 architecture to implement an interrupt vector table. 
its called Interrupt Vector Table (IVT) in real mode, and called Interrupt Descriptor Table (IDT) in protected mode.


https://en.wikipedia.org/wiki/Interrupt_descriptor_table
https://wiki.osdev.org/Interrupt_Descriptor_Table

Use of the IDT is triggered by three types of events: hardware interrupts, software interrupts, and processor exceptions, which 
together are referred to as interrupts.



-   Real mode
in the 8086 processor, the interrupt table is called IVT (interrupt vector table). Its really the same thing as IDT
but just called differently if you are in real mode. 
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
                |     IVT           |
                |   Interrupt Table |
                |                   |
                |                   |
                |                   |
                |___________________|   <--------- 0x000000  

a real mode pointer is defined as a 16-bit segment and a 16-bit offset into that segment. 


so if you look at the Understanding Linux Kernel book,
https://doc.lagout.org/operating%20system%20/linux/Understanding%20Linux%20Kernel.pdf
Chapter 4, Preliminary Initialization of the IDT

        The IDT is initialized and used by the BIOS routines while the computer still operates in Real Mode. 
        Once Linux takes over, however, the IDT is moved to another
        area of RAM and initialized a second time, because Linux does not use any BIOS routine


-   Protected mode
in protected mode, the IDT can reside anywhere in the linear address space.
The location of IDT (address and size) is kept in the IDTR register of the CPU, which can be loaded/stored using LIDT, SIDT instructions. 

quoting the intel manual: 
        "This instruction can be executed only when the CPL is 0. It normally is used by the initalization code 
        of an OS when creating an IDT"


the table contains 8-byte Gates entries. Each entry has a complex structure.


                struct IDTDescr {
                   uint16_t offset_1; // offset bits 0..15
                   uint16_t selector; // a code segment selector in GDT or LDT
                   uint8_t zero;      // unused, set to 0
                   uint8_t type_attr; // type and attributes, see below
                   uint16_t offset_2; // offset bits 16..31
                };




https://linux-kernel-labs.github.io/refs/heads/master/lectures/interrupts.html
0 ~ 31
first 32 entries are reserved for exceptions 


32 ~ 127
device interrupts



-   Hardware Interrupts 

Pic manages hardware interrupts.

-   Software Interrupts 
software interrupts are triggered by the "INT" instruction. 


-   Exceptions 


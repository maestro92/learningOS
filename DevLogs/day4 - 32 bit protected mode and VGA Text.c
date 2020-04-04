


####################################################################################
############################## real mode vs protected mode #########################
####################################################################################

This is all on the topic of x86 segmentation, the terminology of "real mode" and "protected mode" is all related to x86 segmentation.

A bit of history: 

-   8086
so Segmentation was introduced on the Intel 8086 in 1978 as a way to allow programs to address more than 64 KB (16 bits) of memory 
hence we have the 20 bit addressing of Segment:Offset pairs for memory addressing. 20 bit gives you 1 MB of RAM. 
if you think about it, any address can be represented by up to 4096 distinc segment:offset pairs. 

when the memory segmentation was first developed, it does not provide any protection. Any program running on these processors can access 
any segment with no restrictions. 

As the cost of memory use increased, the 1 MB limiitation became a signification problem Intel intended to solve 
this limitation along with others with the release of 80286. 


-   80286
The intel 80286 introduced a 2nd version of segmentation in 1982 that added support for virtual memory and memory protection.
So its was the first x86 equipped with a memory management unit (MMU), allowing virtual memory.
At this point the original model was renamed "real mode", and the new version was named "protected mode". 

So Segmentation in the Intel 80286 and later provides protection, and at this point, Intel named the previous mode "real mode" and 
this new mode "protected mode" with protection features.   

Apparently the "protected mode" is also called virtual memory mode. In lay man_s terms, a virtual memory machine is just a machine 
that maps a larger memory space (1G for the 80286) into a much smaller physical memory space (16M bytes for 80286). 


-   80386
Then intel introduced 80386 in 1985. 80386 was like the 32-bit extension of the 80286 architecture. In Intel 80386 and later processors 
also support paging. Its protected mode retains the segmentation mechnaism of 80286 protected mode. 80386 had 32 bits, which allows 
for 2^32 bytes of memory, which is 4 GB. 

So a paging unit was added as a second layer of address translation between the segmentation unit and the physical bus. 
In those processors, the segment table, rather than pointing to a page table for the segment, contains the segment address in linear memory. 
Addresses in linear memory are then mapped to physical addresses using a separate page table, if paging is enabled.

If paging unit is enabled, addresses in a segment are now virtual addresses, rather than physical addresses as they were on the 
80286. The 80386 also introduced two new general-purpose data segment registers, FS and GS. 



the x86-64 architecture, introduced in 2003, has largely dropped support for segmentation in 64-bit mode.
In a x86-64 architecture it is considered legacy and most x86-64-based modern system software dont use memory segmentation.
Four of the segment registers: CS, SS, DS, and ES are forced to 0, and the limit to 2^64. 
The segment registers FS and GS can still have a nonzero base address. Unlike the global descriptor table mechanism used by legacy modes,
the base address of these segments is stored in a model-specific register.

To support old software, the processor starts up in "real mode", a mode in which it uses the segmented addressing model of the 8086. 

https://en.wikipedia.org/wiki/X86_memory_segmentation
https://en.wikipedia.org/wiki/Protected_mode



-   Segment registers in real mode 

In both real and protected modes, the system uses 16-bit segment registers to derive the actual memory address. In real mode, 
the registers CS, DS, SS and ES point to the currently used program code segment (CS), the current data segment (DS),
the current stack segment (SS), and one extra segment determined by the programmer (ES). 

-   Segment registers in protected mode 

The 80286_s protected mode extends the processor_s address space to 224 bytes (16 megabytes), but not by adjusting the shift value. 
Instead, the 16-bit segment registers now contain an index into a table of segment descriptors containing 
24-bit base addresses to which the offset is added.




####################################################################################
############################## 32 bit protected mode ###############################
####################################################################################

continuing on the tutorial, we look at Chapter 4
https://www.cs.bham.ac.uk/~exr/lectures/opsys/10_11/lectures/os-dev.pdf

As mentioned, from day2, we have been working in 16 bit real mode, now we want to go into 32-bit protected mode 
Essentially we want to do the thing that was introduced in 80386. So 80286 was still a 16-bit processor,
80386 was 32bit. So that gives you an idea of what we are doing. 


The reason why we want to go into 32 bit protected mode is because: 

-   registers are extended to 32 bits, so ax, bx, cx, dx, becomes eax, ebx, ecx, edx ...

-   you also get two more segment registers, fs and gs 

-   CPU can now implement virtual memory for user processes

-   interrupt handling is more sophisticated. 


Essentially 32-bit mode allows us to use 32 bit registers and memory addressing, protected memory, virtual memory and other 
advantages, but we will lose BIOS interrupts and we will need to code the GDT, global descriptor table

real mode provides no support for memory protection, multitasking, or code privilege levels 

Protected mode is also called protected virtual address mode. 

So that is what we want to do 





-   GDT
So as mentioned, we will need to prepare a complex data structure in memory called the global descriptor table (GDT),
which defines memory segments and thir protected-mode attributes. 

So intel defines 3 types of tables, the Interrupt Descriptor Table (IVT), the Global Descriptor Table (GDT) 
and the Local Descriptor Table. Each table is defined as a (size, linear address) to the CPU through the 
LIDT, LGDT, LLDT instructions respectively. In most cases, the OS simply tells where those tables rae once at boot time,
and then simply goes writing/reading the tables through a pointer. 


So the the Global Descriptor Table is specific to the x86-based processors architectures. Its essentially a table that describes 
info about the memory such as protected-mode attributes. We will learn more about this tomorrow 

https://wiki.osdev.org/Global_Descriptor_Table



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


Since we lose all BIOS routines, we have to provide our own drivers for all hardware of the machine (e.g. the keyboard, 
screen, disk drivers, mouse, etc). 


###############################################################################
########################## Printing In Protected Mode #########################
###############################################################################
so althought we are not yet in 32-bit protected mode, we need a way to have the screen print stuff out or 
interact with the user (essentially give the users some sort of feedback) that works in 32-bit protected mode. 
Recall that once we are in protected mode, BIOS interrupts stops woorking. So calling int 0x10 no longe works 

To solve this problem, we can use VGA text mode. VGA text mode still works in 32-bit protected mode. When most computers boot, 
desprite tha they may infact have more advanced graphics hardware, they begin in a simple VGA color text mode with 80 x 25 characters

In text mode, the programmer does not need to render individual pixels to describe characters, since a simple 
font is already defined in the internal memory of the VGA display device. So we get to avoid manipulating direct pixels. 


The VGA memory starts at address 0xb8000. So to give you an idea where 0xb8000 is at, 

You can refer to this graph
https://www.yourdictionary.com/pc-memory-map
or this one
https://www.slideserve.com/Olivia/pc-bootup

lets manually draw a memory map. lets assume we are 32 bit, which gives us 4GB of memory  

32 bit computers support 4GB cuz you can represent numbers up to [0, 2^32 - 1] 

2^32 is 4294967296 

hexidecimal means base 16. So each hex decimal represents 4 bits 

F is 1111

I was a bit rusty, so I had to brush up on some binary and hex math just to give some reference in bytes 

                    in Hex 
1 KB        2^10    400
1 MB        2^20    100000
1 GB        2^30    40000000



so 0xB8000 is 1011100000 0000000000 in binary
1011100000 is 736 

so this is 736 kb



so 0xBC000 is 1011110000 0000000000 in binary 

1011110000 is 752 
so this is 752 kb


as you can see, the VGA Text memory resides between 736 kb and 752 kb


                 ___________________    <--------- 0x00000000 
                |                   |
                |                   |
                |                   |
                |___________________|   <-------- 0x00007C00 (31 kb)
                |    bootsector     |
                |                   |   
                |___________________|  
                |                   |
                |                   |
                |                   |   
                |                   |
                |                   |   
                |                   |
                |___________________|   <--------- 0x000B8000 (736 kb)
                |                   |
                |   CGA Graphics    |
                |   CGA, EGA &      |
                |   VGA Text        |
                |___________________|   <--------- 0x000BC000 (752 kb)
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
                |                   |   <--------- 0x100000 (1MB)


https://manybutfinite.com/post/motherboard-chipsets-memory-map/

So this brings up to to the topic of memory map are also used for communication with assorted devices on the motherboard. 
this communication is called memory-mapped I/O (https://en.wikipedia.org/wiki/Memory-mapped_I/O)

These devices include video cards, most PCI cards (say a scanner, or SCSI card), and also flash memory that stores he BIOS 


So when the mother board receives physical memory requests, it decides where to rout it. 
The routing is decided via the memory address map. For each region of physical memory addresses, the memory map knows 
the device that owns the region. 

The link above has an image and shows the memory-mapped I/O.

Notice in the graph, the amount of actual RAM available is around 3GB




-   video memory on bootup
https://stackoverflow.com/questions/20861032/who-loads-the-bios-and-the-memory-map-during-boot-up

So on Day2, we talked about how BIOS calls into the bootstrap code. Today we will talk about memory map on boot up.


Recall that on day 2, we mentioned that BIOS copies itself into RAM, as you can image, BIOS also initializes the memory 
controller before it copies itself from ROM to RAM. 

The memory map layout varies from system to system. At power-on, the BIOS will query the attached PCI/PCIe devices,
determine what resources are needed and place them in the memory map at the optimcal location. 
If everything is working properly, memory-mapped devices should not overlap with RAM. 

Essentially people intel decides that for a certain model of computers, they bake the memory map information into the chipset 
and the BIOS knows about the memory map. 





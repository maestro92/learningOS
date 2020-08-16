####################################################################################
############################## real mode vs protected mode #########################
####################################################################################
So after talking about bootsectors, we need to talk about processor modes. Essentially real mode vs protected mode


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

the Intel 80286 is a 16-bit processor with a segment-based memory management and protection system.


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

in summary, real mode 
-   uses the native segment:offset memory model 
-   is limited to 1 MB of memory
-   no virtual memory or memory protection


-   Segment registers in protected mode 

The 80286_s protected mode extends the processor_s address space to 224 bytes (16 megabytes), but not by adjusting the shift value. 
Instead, the 16-bit segment registers now contain an index into a table of segment descriptors containing 
24-bit base addresses to which the offset is added.



####################################################################################
############################## real vs protected mode ##############################
####################################################################################

-   Real mode has no memory protection. All data and code are dumped into a single all purpose use memory block. 
-   In real mode, you are limited to 16 bit registers. That means youare limited to 1 MB of fmemory 
-   No support for hardware level memory protection or multitasking
-   Quite possibly the biggest problem, was that there is no such thing as "rings". 
    All programs execute at Ring 0 level, as every program has full control over the system. 
    This means, in a single tasking envirement, a single instruction (such as cli/hlt) can crash the entire OS if you are not carefull.

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

so real mode gives you 
-   memory protection
-   hardware support for Virtual Memory and Task State Switching (TSS)
-   Hardware support for interrupting programs and executing another 
-   4 Operating Modes: Ring 0, 1, 2, 3
-   Access to 32 bit registers, which means also 4 GB of memory 



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


this links actually offers a ver good graph
https://manybutfinite.com/post/how-computers-boot-up/



###############################################################################
############################## Memory #########################################
###############################################################################

So we first need to talk about how memory works 








##################################################################################
############################### Memory Map #######################################
##################################################################################

x86 Real Mode Memory Map

0x00000000 - 0x000003FF - Real Mode Interrupt Vector Table
0x00000400 - 0x000004FF - BIOS Data Area
0x00000500 - 0x00007BFF - Unused
0x00007C00 - 0x00007DFF - Our Bootloader
0x00007E00 - 0x0009FFFF - Unused

0x000A0000 - 0x000BFFFF - Video RAM (VRAM) Memory
    0x000B0000 - 0x000B7777 - Monochrome Video Memory
    0x000B8000 - 0x000BFFFF - Color Video Memory

0x000C0000 - 0x000C7FFF - Video ROM BIOS
0x000C8000 - 0x000EFFFF - BIOS Shadow Area

0x000F0000 - 0x000FFFFF - System BIOS

Note: 
To convert 0x000A0000 into the numeric kb 
you can just put 0x000A0000 into math is fun, find out the decimal amount which is 655360.
then divide it by 1024. Then you get the 640. So you know is 640 kb.


Lets draw a graph

                 _______________________________    <--------- 0x100000 (1MB)
                |                               |
                |                               |
                |         System BIOS           | 
                |                               |
                |                               |
                |_______________________________|   <--------- 0xF0000 (960 kb) 
                |                               |
                |       BIOS Shadow Area        |
                |_______________________________|   <--------- 0xC8000 (800 kb) 
                |                               |
                |       Video ROM BIOS and      |
                |_______________________________|   <--------- 0xC0000 (768 kb)
                |                               |
                |   Monochrome Video Memory and |   
                |       Color Video Memory      |
                |_______________________________|   <--------- 0xB0000 (704 kb)
                |                               |
                |         Legacy Video          |
                |       Card Memory Access      |
                |_______________________________|   <--------- 0xA0000 (640 kb)
                |                               |
                |                               |   
                |                               |
                |                               |               
                |                               |
                |                               |
                |                               |
                |                               |               
                |                               |
                |_______________________________|   
                |                               |   
                |           bootsector          |   
                |_______________________________|   <-------- 0x7C00 (31 kb)              
                |                               |
                |                               |
                |                               |
                |                               |
                |                               |   
                |                               |   
                |_______________________________|   <--------- 0x00000000 


so this gives you an idea of how our memory looks like



Recall its possible to change the address range used by the Graphics Controller. When initializing, the BIOS 
does just this to map video memory to 0xA000



#########################################################################################
########################## Protected Mode Memory Addressing #############################
#########################################################################################

So how does memory addressing work in protected mode?

So OSDev7 offers a good explanation.
http://www.brokenthorn.com/Resources/OSDev7.html

if you scrow down there is a section called "How instructions Execute"


so the IP register contains the offset address of the currently executing instruction.
CS contains the segment address.

so the processor first calculates the absolute address that it needs to read from.
If you recall the segment:offset model, the absolute address is 
        

        absolute address = segment * 16 + offset 

so if you are refering to regisers then, it will be 

        aboslute address = CS * 16 + IP



OSDev8 also has a section "PMode Memory Addressing"

This means, in order to access memory in PMode, we have to go through the correct descriptor in the GDT. 
The descriptor is stored in CS. This allows us to indirectly refrence memory within the current descriptor.



###############################################################################
############################## Enabling A20  ##################################
###############################################################################


A little history
When IBM designed the IBM PC AT machines, it used their newer Intel 80286 microprocessor, 
which was not entirely compatible with previous x86 microprocessors when in real mode. 
The problem? The older x86 processors did not have address lines A20 through A31. 
They did not have an address bus that size yet. Any programs that go beyond the first 
1 MB would appear to wrap around. While it worked back then, the 80286s address space required 32 address lines. 
However, if all 32 lines are accessable, we get the wrapping problem again.

To fix this problem, Intel put a logic gate on the 20th address line between the processor and system bus. This logic gate got named Gate A20, as it can be enabled and disabled. For older programs, it an be disabled for programs that rely on the wrap wround, and enabled for newer programs.

When booting, the BIOS enables A20 when counting and testing memory, and then disables it again before giving our operating system control.


A20, or addressing line 20

The Intel 8086, Intel 8088, and Intel 80186 processors had 20 address lines, 
numbered A0 to A19; with these, the processor can access 220 bytes, or 1 MB. 
Internal address registers of such processors only had 16 bits. 


The 80286 could address up to 16 MB of system memory in protected mode.




###############################################################################
########################## Getting amount of RAM  #############################
###############################################################################

so in real mode, we also want to get the amount of RAM 
if you look at the wikipage 
    https://en.wikipedia.org/wiki/BIOS_interrupt_call

and you look at 12h, you can it says 

                12h         Return conventional memory size

so we will do that.







###############################################################################
########################## Rolling your bootloader ############################
###############################################################################

Where will you load your kernel?
You will have to decide where in memory you are going to load your kernel. Your kernel generally depends on it.

In Real Mode, the easiest is to stay below the 1MB barrier, which means you practically have 512KB of memory to load things. 
You may wish the kernel to be loaded at a well-known position, say 0x10000 physical (es=0x1000, bx=0 when calling INT13h).

If your kernel is bigger (or is expected to grow bigger) than this, you wll probably prefer to have the 
kernel above the 1MB barrier, which means you need to activate A20 gate and switch to Unreal Mode to 
load the kernel (with A20 alone, you cannot have more than 64K above 1MB).

Note that BIOS will still be unable to write to memory above 1MB, so you need to read stuff in a buffer below 
1MB and then perform a rep movsd to place the data where they ultimately should go.


If you are loading above 1MB, you should proceed in 2 steps: first using BIOS to load in the "conventional" area, 
and then performing a rep movsd to place the data where they ultimately should go.
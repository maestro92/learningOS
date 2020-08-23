
##################################################################
################# Memory Management Unit #########################
##################################################################

So a memory managment unit is a computer hardware unit having all memory references passed through itself,
primarily performing the translation of virtual memory addresses to physical addresses. 

An MMU effectively performs virtual memory management, handling at the same time memory protection, cache control, 
bus arbitration and, in simpler computer architectures (especially 8-bit systems), bank switching.

Some architectures have the MMU built-in with the CPU, while others have a separate chip.

So having multiple address spaces allows each task to have its own memory space to work in.
In modern systems, this is a major part of memory protection. Keeping processes_s memory 
spaces separate allows them to run without causing problems in another process_s memory space.











Virtual memory is dependent o nhardware support. It cannot be emulated by software


So in this link, it contains an image at the bottom 
http://faculty.salina.k-state.edu/tim/ossg/Memory/mem_hw.html

link to image 
http://faculty.salina.k-state.edu/tim/ossg/_images/mmu.png

it illustrates how the MMU works with the CPU and talks to the memory chip. As you can see the MMU 
is on the CPU, and whenever the CPU talks to the memory chip, it will go through the MMU. 







The next step is, instead of reporting an "out of memory" once the physical memory runs out, 
is to take pages that are not actually accessed currently, and write them to hard disk (swapfile or -partition) - 
freeing up the physical memory page. This is referred to as "paging out" memory.




https://cirosantilli.com/x86-paging

(logical) -------------------> (linear) -------------------> (physical)
            segmentation                    paging







virtual memory offers memory protection.
with virtual memory, programs dont have to worry about memory fragmentation. 
The OS deals with it, programs address space starts at 0x0000:0000


##################################################################
################# Virtual Memory in Intel ########################
##################################################################


So x86 platform is unique that it uses both segmentation and paging to handle mapping between virtual and physical addresses.



so in day 04 we mentioned that 

        The intel 80286 introduced a 2nd version of segmentation in 1982 that added support for virtual memory and memory protection.
        So its was the first x86 equipped with a memory management unit (MMU), allowing virtual memory.

also from the virutal memory wikipedia article, it says 

        Virtual memory was introduced to the x86 architecture with the protected mode of the Intel 80286 processor.

        The Intel 80386 introduced paging support underneath the existing segmentation layer



        https://en.wikipedia.org/wiki/Virtual_memory



-   Intel 80386
        The 80386 added a three-stage instruction pipeline, extended the architecture from 16-bits to 32-bits, 
        and added an on-chip memory management unit. This paging translation unit made it much easier to 
        implement operating systems that used virtual memory. It also offered support for register debugging.

        https://en.wikipedia.org/wiki/Intel_80386



x86-64






        Chapter 3.3

        In protected mode, the IA-32 architecture provides a normal physical address space of 4 GBytes (232 bytes). This
        is the address space that the processor can address on its address bus. This address space is flat (unsegmented),
        with addresses ranging continuously from 0 to FFFFFFFF (8 x F). This physical address space can be mapped to readwrite memory, 
        read-only memory, and memory mapped I/O. The memory mapping facilities described in this
        chapter can be used to divide this physical memory up into segments and/or pages.
        Starting with the Pentium Pro processor, the IA-32 architecture also supports an extension of the physical address
        space to 236 bytes (64 GBytes); with a maximum physical address of FFFFFFFFF (9 x F). This extension is invoked in
        either of two ways:
        • Using the physical address extension (PAE) flag, located in bit 5 of control register CR4.
        • Using the 36-bit page size extension (PSE-36) feature (introduced in the Pentium III processors).
        Physical address support has since been extended beyond 36 bits. See Chapter 4, “Paging” for more information
        about 36-bit physical addressing.

https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.html





On the x86-64 architecture, page-level protection now completely supersedes Segmentation as the memory protection mechanism.



######################################################################
########################### Translating ##############################
######################################################################

To translation of linear addresses to physical addresses is described in the figure below:

https://littleosbook.github.io/images/intel_4_2_linear_address_translation.png




The 20 bits pointing to the current PDT is stored in the register cr3. 








When PAE paging is used, CR3 references the base a 32-byte page-directory-pointer table. 







######################################################################
########################### Paging ###################################
######################################################################

So apparently there is 3 modes of paging. 
We will do the 32-bit paging.


The 20 bits pointing to the current PDT is stored in the register cr3. The lower 12 bits of cr3 are used for configuration.

For more details on the paging structures, see chapter 4 in the Intel manual [33]. 
The most interesting bits are U/S, which determine what privilege levels can access this page (PL0 or PL3), 
and R/W, which makes the memory in the page read-write or read-only.





The format 
https://wiki.osdev.org/Paging

Both tables contain 1024 4-byte entries, making them 4Kb each. 

Each page directory entry points to a page table, 
Each page table entry points to a physical address 


https://pdos.csail.mit.edu/6.828/2005/readings/i386/s05_03.htm
https://pdos.csail.mit.edu/6.828/2005/readings/i386/fig5-13.gif


https://littleosbook.github.io/#paging
Chapter 9


Page Directory 


Page Table 


Paging is the most common technique used in x86 to enable virtual memory. 
Virtual memory through paging means that each process will get the impression that the available memory range is 0x00000000 - 0xFFFFFFFF

The linear address gets translated to a physical address by the MMU and the page table. 
If the virtual address isn’t mapped to a physical address, the CPU will raise a page fault interrupt.


Page Directory Table (PDT)
Page Table (PT) 
Page Frame (PF)

all page PDT, PT and PF need to be aligned 4096 bytes. This make sit possible to address it with just the highest 20 bits of a 32 bit address. 


3.  

                u32int kmalloc(u32int sz, int align)
                {
                    if (align == 1 && (placement_address & 0xFFFFF000)) // If the address is not already page-aligned
                    {
                        // Align it.
                        placement_address &= 0xFFFFF000;
                        placement_address += 0x1000;
                    }
                    u32int tmp = placement_address;
                    placement_address += sz;
                    return tmp;
                }




######################################################################
############################## PAE ###################################
######################################################################

Physical address extension (PAE) flag, located in bit 5 of control register CR4.









The following assembly code shows an example:



                ; eax has the address of the page directory
                mov cr3, eax

                mov ebx, cr4        ; read current cr4
                or  ebx, 0x00000010 ; set PSE
                mov cr4, ebx        ; update cr4

                mov ebx, cr0        ; read current cr0
                or  ebx, 0x80000000 ; set PG
                mov cr0, ebx        ; update cr0

                ; now paging is enabled



                void init_paging()
                {

                }



                void * get_physaddr(void * virtualaddr)
                {
                    unsigned long pdindex = (unsigned long)virtualaddr >> 22;
                    unsigned long ptindex = (unsigned long)virtualaddr >> 12 & 0x03FF;
                 
                    unsigned long * pd = (unsigned long *)0xFFFFF000;
                    // Here you need to check whether the PD entry is present.
                 
                    unsigned long * pt = ((unsigned long *)0xFFC00000) + (0x400 * pdindex);
                    // Here you need to check whether the PT entry is present.
                 
                    return (void *)((pt[ptindex] & ~0xFFF) + ((unsigned long)virtualaddr & 0xFFF));
                }



                void map_page(void * physaddr, void * virtualaddr, unsigned int flags)
                {
                    // Make sure that both addresses are page-aligned.
                 
                    unsigned long pdindex = (unsigned long)virtualaddr >> 22;
                    unsigned long ptindex = (unsigned long)virtualaddr >> 12 & 0x03FF;
                 
                    unsigned long * pd = (unsigned long *)0xFFFFF000;
                    // Here you need to check whether the PD entry is present.
                    // When it is not present, you need to create a new empty PT and
                    // adjust the PDE accordingly.
                 
                    unsigned long * pt = ((unsigned long *)0xFFC00000) + (0x400 * pdindex);
                    // Here you need to check whether the PT entry is present.
                    // When it is, then there is already a mapping present. What do you do now?
                 
                    pt[ptindex] = ((unsigned long)physaddr) | (flags & 0xFFF) | 0x01; // Present
                 
                    // Now you need to flush the entry in the TLB
                    // or you might not notice the change.
                }




//https://wiki.osdev.org/Setting_Up_Paging














######################################################################
########################### Page Directory ###########################
######################################################################

So visually in memory, your page directory + page tables looks like below:
http://www.jamesmolloy.co.uk/tutorial_html/6.-Paging.html


                 ___________________    <--------- 0x00000000 
                |                   |
                |                   |
                |                   |
                |___________________|   <--------- somewhere in memory
                |                   |
                |  page directory   |   1024 * 4 bytes
                |___________________|
                |                   |
                |   page table 0    |   1024 * 4 bytes
                |_ _ _ _ _ _ _ _ _ _|
                |                   |
                |   page table 1    |
                |_ _ _ _ _ _ _ _ _ _|
                |                   |
                |   page table 2    |
                |_ _ _ _ _ _ _ _ _ _|
                |                   |
                |   page table 3    |
                |_ _ _ _ _ _ _ _ _ _|
                |                   |                
                |        ...        |
                |___________________|         
                |                   |
                |  page table 1023  |
                |___________________|
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
                |___________________|   <---------- 4 GB (not drawn to scale)

so the total math requried is 

each page table is 1024 * 4 bytes = 4kb
with 1024 page table, we have 1024 * 4 kb = 4 MB

so total storage is 4 MB + 4 kb.





This is assuming every page table is present in the page directory. If your system doesnt need to use that many 
memory, then you can set up less tables 


                 ___________________    <--------- 0x00000000 
                |                   |
                |                   |
                |                   |
                |___________________|
                |                   |
                |  page directory   |   1024 * 4 bytes
                |___________________|
                |                   |
                |   page table 0    |   1024 * 4 bytes
                |_ _ _ _ _ _ _ _ _ _|
                |                   |
                |   page table 1    |
                |_ _ _ _ _ _ _ _ _ _|
                |                   |
                |   page table 2    |
                |_ _ _ _ _ _ _ _ _ _|
                |                   |
                |   page table 3    |
                |___________________|
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
                |___________________|   

assuming you are only using 4 tables, then the storage used is 


each page table is 1024 * 4 bytes = 4kb
with 4 page table, we have 4 * 4 kb = 16kb

so total storage used is 20kb.




######################################################################
########################### Enabling Paging ##########################
######################################################################

Paging is enabled by first writing the address of a page directory to cr3 and then setting bit 31 
(the PG “paging-enable” bit) of cr0 to 1. To use 4 MB pages, set the PSE bit (Page Size Extensions, bit 4) of cr4. 

Enabling paging is actually very simple. All that is needed is to load CR3 with the address of the page directory and
to set the paging (PG) and protection (PE) bits of CR0.

Copy the location of your page directory into the CR3 register. This must, of course, be the physical address.
Set the PG bit in the CR0 register. You can do this by OR-ing with 0x80000000.





######################################################################
############################ Page Directory ##########################
######################################################################

Lets code them up. First define this table.
So we first set every

                unsigned int page_directory[1024] __attribute__((aligned(4096)));

                void initPaging()
                {

                    for(int i=0; i<1024; i++)
                    {
                        // This sets the following flags to the pages:
                        //   Supervisor: Only kernel-mode can access them
                        //   Write Enabled: It can be both read from and written to
                        //   Not Present: The page table is not present
                        page_directory[i] = 0x00000002;
                    }
                 
                }


so the page directory has a bunch of page table entries. 
the specs are: 

https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.html

one of the table is

                Table 4-5. Format of a 32-Bit Page-Directory Entry that References a Page Table


so our flags are page_directory[i] = 0x00000002;




So next let us try to set up our first page table 
againt our page table is just an array of 1024 uint32_t
                
                unsigned int first_page_table[1024] __attribute__((aligned(4096)));

So here we want to map the pages to the beginning of memory 0 ~ 4MB

                for(int i=0; i<1024; i++)
                {
                    first_page_table[i] = (i * 0x1000) | 3;
                }








in the JamesM Tutorial, apparently, you can define a struct and its bit fields 

                typedef struct page
                {
                    u32int present    : 1;   // Page present in memory
                    u32int rw         : 1;   // Read-only if clear, readwrite if set
                    u32int user       : 1;   // Supervisor level only if clear
                    u32int accessed   : 1;   // Has the page been accessed since last refresh?
                    u32int dirty      : 1;   // Has the page been written to since last refresh?
                    u32int unused     : 7;   // Amalgamation of unused and reserved bits
                    u32int frame      : 20;  // Frame address (shifted right 12 bits)
                } page_t;







again, to look at the page-table entry, you can look at the intel specs,
Table 4-6. Format of a 32-Bit Page-Table Entry that Maps a 4-KByte Page

https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.html

But there is a more visualized version at JamesM tutorial page at chapter 6.2
http://www.jamesmolloy.co.uk/tutorial_html/6.-Paging.html


        P
            Set if the page is present in memory.
        R/W
            If set, that page is writeable. If unset, the page is read-only. This does not apply when code is running in kernel-mode (unless a flag in CR0 is set).
        U/S
            If set, this is a user-mode page. Else it is a supervisor (kernel)-mode page. User-mode code cannot write to or read from kernel-mode pages.
        Reserved
            These are used by the CPU internally and cannot be trampled.
        A
            Set if the page has been accessed (Gets set by the CPU).
        D
            Set if the page has been written to (dirty).
        AVAIL
            These 3 bits are unused and available for kernel-use.
        Page frame address
            The high 20 bits of the frame address in physical memory.

so here we are only setting the writeable bit. 

so for us 

P:      is set cuz we are initializing the pages for our first table 
R/W:    we set it to writeable 
U/S:    we set it to kernel mode 
A:      we are just initializing our pages, so its zero, not yet accessed
D:      we havent really touched our pages, so its unset 

so our flags are 0x00000003
0x1000 is 4KB, so we are setting 4kb pages. 
so combining the flags and the page address, we get (i * 0x1000) | 3.




then we put the page table in the Page Directory.

        page_directory[0] = ((unsigned int)first_page_table) | 3;



-   Enable Paging 
as mentioned, we need to do two things, set the values in cr3 and cr0.

The final step is to actually enable paging. First we tell the processor where to find our page directory by putting its 
address into the CR3 register. Because C code cannot directly access the computer_s registers, 
we will need to use assembly code to access CR3. 

                


This small assembly function takes one parameter: the address of the page directory. It then loads the address onto the CR3 register, 
where the MMU will find it. But wait! Paging is not still enabled. Thats what we will do next. 
We must set the 32th bit in the CR0 register, the paging bit. This operation also requires assembly code. Once done, paging will be enabled.



######################################################################
################################# Page ###############################
######################################################################

P
    Set if the page is present in memory.
R/W
    If set, that page is writeable. If unset, the page is read-only. This does not apply when code is running in kernel-mode (unless a flag in CR0 is set).
U/S
    If set, this is a user-mode page. Else it is a supervisor (kernel)-mode page. User-mode code cannot write to or read from kernel-mode pages.
Reserved
    These are used by the CPU internally and cannot be trampled.
A
    Set if the page has been accessed (Gets set by the CPU).
D
    Set if the page has been written to (dirty).
AVAIL
    These 3 bits are unused and available for kernel-use.
Page frame address
    The high 20 bits of the frame address in physical memory.















######################################################################
############################ TLB #####################################
######################################################################

The Translation Lookahead Buffer (TLB) is a cache for paging addresses.

This is a cache stored within the processor used to improve the speed of virtual address translation.




So we got Paging set up, we need to do the page fault handler

https://wiki.osdev.org/Paging


so the way we handle a page fault is through interrupts.
so if you recall intel sepcs, 
https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.html
chapter 6.15, 

Interrupt 14 is the Page-Fault Exception

So we need to setup the interrupt handler for interrupt 14.



########################################################################
############################ Handler ###################################
########################################################################



                void page_fault_handler(registers_info* info)
                {   
                   // A page fault has occurred.
                   // The faulting address is stored in the CR2 register.
                   unsigned int faulting_address;
                   asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

                   // The error code gives us details of what happened.
                   int present   = !(info->err_code & 0x1); // Page not present
                   int rw = info->err_code & 0x2;           // Write operation?
                   int us = info->err_code & 0x4;           // Processor was in user-mode?
                   int reserved = info->err_code & 0x8;     // Overwritten CPU-reserved bits of page entry?
                   int id = info->err_code & 0x10;          // Caused by an instruction fetch?

                   // Output an error message.
                   kprint("Page fault! ( ");
                   if (present) {kprint("page not present ");}
                   if (rw) {kprint("read-only ");}
                   if (us) {kprint("user-mode ");}
                   if (reserved) {kprint("reserved ");}
                   kprint(") at ");
                   kprint_hex(faulting_address);
                   kprint("\n");

                   for(;;);
                }




CR2 register

    Linear Address that generated the exception 


    https://pdos.csail.mit.edu/6.828/2004/lec/lec8-slides.pdf




########################################################################
############################ Testing ###################################
########################################################################

so to to test paging, you can have something like this

                void test_paging()
                {
                   unsigned int *ptr = (unsigned int*)0x00000FFF;
                   unsigned int do_page_fault = *ptr;

                   unsigned int *ptr2 = (unsigned int*)0xB0000000;
                   do_page_fault = *ptr2;
                }





#####################################################################################
############################### Higher Half Kernels #################################
#####################################################################################

A Higher Half Kernel is a kernel that has a virtual base address of 2GB or above. 

The Windows Kernel gets mapped to either 2GB or 3GB virtual address (depending on if /3gb kernel switch is used), 
the Linux Kernel gets mapped to 3GB virtual address. 

The series uses a higher half kernel mapped to 3GB. 
Higher half kernels must be mapped properly into the virtual address space. 
There are several methods to achieve this, some of which is listed here.

You might be interested on why we would want a higher half kernel. 
We can very well run our kernel at some lower virtual address. 
One reason has to do with v86 tasks. If you want to support v86 tasks, 
v86 tasks can only run in user mode and within the real mode address limits (0xffff:0xffff), 
or about 1MB+64k linear address. It is also typical to run user mode programs 
in the first 2GB (or 3GB on some OSs) as software typically never has a need to access high memory locations.







Identity Mapping
Identity Mapping is nothing more then mapping a virtual address to the same physical address. 
For example, virtual address 0x100000 is mapped to physical address 0x100000. 
Yep--Thats all there is to it. The only real time this is required is when first setting up paging. 
It helps insure the memory addresses of your current running code of where they 
are at stays the same when paging is enabled. Not doing this will result in immediate triple fault.
You will see an example of this in our Virtual Memory Manager initialization routine.













https://wiki.osdev.org/Memory_Allocation

At square one, the kernel is the only process in the system. But it is not alone: BIOS data structures, 
memory-mapped hardware registers etc. populate the address space. Among the first things a kernel must do 
is to start bookkeeping about which areas of physical memory are available for use and which are to be considered "occupied".

The free space will subsequently be used for kernel data structures, application binaries, their heap and stack etc. - 
the kernel needs a function that marks a memory area as reserved, and makes that memory available to the process requiring it. 
In the C Standard Library, this is handled by malloc() and free(); in C++ by new() and delete().


Page Frame Allocator



For managing things that arent RAM, you need a map of the physical address space that says which areas are used for what. 
For example, when you are trying to initialize a new PCI device (to configure its "BARs") you want to search for an area that 
isnt ROM, isnt used by another device and isnt RAM. This information includes "cacheability attributes" and is used to configure
 (and manage) the CPU_s MTRRs. The physical address space map should also contain information 
 for RAM - e.g. is it volatile or non-volatile, is it hot-pluggable, etc.







9.3.2 The Virtual Address for the Kernel
Preferably, the kernel should be placed at a very high virtual memory address, for example 0xC0000000 (3 GB). 
The user mode process is not likely to be 3 GB large, which is now the only way that it can conflict with the kernel. 
When the kernel uses virtual addresses at 3 GB and above it is called a higher-half kernel. 
0xC0000000 is just an example, the kernel can be placed at any address higher than 0 to get the same benefits. 
Choosing the correct address depends on how much virtual memory should be available 
for the kernel (it is easiest if all memory above the kernel virtual address should belong to the kernel) 
    and how much virtual memory should be available for the process.

If the user mode process is larger than 3 GB, some pages will need to be swapped out by the kernel. 
Swapping pages is not part of this book.



as soon as paging is enabled, all address become virtual. To fix this, we must map the virtual addresses to the same physical 
address so they refer to the same thing. This is identity mapping 

For example, lets say we want to write to video memory or any other memory-mapped devices, the memory address
for these are exact. So once we enabled paging, we still need to properly refer to these addresses.

So the simple thing here to do is to map the first 1MB properly





each table represents a full 4MB virtual address space. 





https://os.phil-opp.com/advanced-paging/


When the CPU first turns on the "paging" capability, it must be executing code from an 'identity-mapped' page, otherwise it crashes 

since we are running the kernel code, that means our kernel first needs to be identity mapped 


Besides having at least one page that is 'identity-mappedt' (for turning 'paging' on), there can be multiple other mappings

when paging is off, virtual address = physical address. 

The kernel sets up a small identity mapping (virtual == physical) as well as a the kernel direct mapping. 
The identity mapping is needed for the code that turns on paging. 


also lets say we actually want to manipulate the actual page tables 

For example, lets say we have already enabled paging, then lets say a memory request from a process comes in 
and then we need to edit the "present" or "dirty" bit in the page table. 

That means we have to actually access the page table entry from that page table 
and then change the "dirty" bit. 

but in order to access that page table entry, we have to access it from its memory address.

So we need to somehow know the page table addresses virtual memory 

solution 1 is 
which means we need to identity map all page tables and the page directory.


solution 2 is that you can have some math scheme. 
For example, any memory address - 1MB is the mapping from page table virtual memory to page table physical memory.


In this example, we see various identity-mapped page table frames. 
This way the physical addresses of page tables are also valid virtual addresses so that we can 
easily access the page tables of all levels starting from the CR3 register.



we also want to map kernel to 3GB virtual address. 

page directory 


                #define PAGE_DIRECTORY_INDEX(x) (((x) >> 22) & 0x3ff)
                #define PAGE_TABLE_INDEX(x) (((x) >> 12) & 0x3ff)
                #define PAGE_GET_PHYSICAL_ADDRESS(x) (*x & ~0xfff)


0x3ff is 001111111111

so a 32 bit address is 32 bit 
we shift 22 bits to get the first 10, which is the first 10 bits 
0000000000 0000000000 000000000000

then & it with 001111111111, so we get the first 10 bits.


and obviously to get the table index, we just get the middle 10 bits



~ is bitwise not 
! is logical not






#####################################################################
########################### triple fault ############################
#####################################################################

When a fault occurs, the CPU invokes an exception handler. If a fault occurs while trying to invoke the exception handler, 
thats called a double fault, which the CPU tries to handle with yet another exception handler. 
If that invocation results in a fault too, the system reboots with a triple fault.

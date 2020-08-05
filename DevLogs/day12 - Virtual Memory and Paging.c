
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

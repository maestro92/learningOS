Detecting Memory


The first thing we need to do is abtain the amount of RAM inside of the computer system. 
There are alot of different ways to do this. Some methods may work on some systems while others may not.

Getting the amount of memory can be very system dependent.


I should point out that there is indeed other methods that we can do to abtain the amount of memory within the system. For example, the CMOS, PnP, and SMBios. 
However the only way to guarantee that you get the correct amount is from the device that configuares it: The BIOS.



BIOS INT 0x12 - Get Memory Size (Conventional Memory)













So here we need to actually properly map virtual memory to usuable physical memory 

recall in day 12, we did 


                unsigned int first_page_table[1024] __attribute__((aligned(4096)));

                for(int i=0; i<1024; i++)
                {
                    first_page_table[i] = (i * 0x1000) | 3;
                }

which maps virtual memory 0 ~ 4MB to physical memory 0 ~ 4MB. This doesnt work cuz 


if you look at understanding the linux kernel, it says 

                page 65

                Physical Memory Layout
                During the initialization phase the kernel must build a physical addresses map that
                specifies which physical address ranges are usable by the kernel and which are
                unavailable (either because they map hardware devices’ I/O shared memory or
                because the corresponding page frames contain BIOS data).
                The kernel considers the following page frames as reserved:
                • Those falling in the unavailable physical address ranges
                • Those containing the kernel’s code and initialized data structures
                A page contained in a reserved page frame can never be dynamically assigned or
                swapped to disk.
                As a general rule, the Linux kernel is installed in RAM starting from the physical
                address 0x00100000—i.e., from the second megabyte. The total number of page frames required depends 
                on how the kernel is configured. A typical configuration
                yields a kernel that can be loaded in less than 3 MB of RAM.
                Why isn’t the kernel loaded starting with the first available megabyte of RAM? Well,
                the PC architecture has several peculiarities that must be taken into account. For

                example:
                • Page frame 0 is used by BIOS to store the system hardware configuration
                detected during the Power-On Self-Test (POST); the BIOS of many laptops,
                moreover, writes data on this page frame even after the system is initialized.

                • Physical addresses ranging from 0x000a0000 to 0x000fffff are usually reserved to
                BIOS routines and to map the internal memory of ISA graphics cards. This area
                is the well-known hole from 640 KB to 1 MB in all IBM-compatible PCs: the
                physical addresses exist but they are reserved, and the corresponding page
                frames cannot be used by the operating system.

                • Additional page frames within the first megabyte may be reserved by specific
                computer models. For example, the IBM ThinkPad maps the 0xa0 page frame
                into the 0x9f one.

                In the early stage of the boot sequence (see Appendix A), the kernel queries the BIOS
                and learns the size of the physical memory. In recent computers, the kernel also
                invokes a BIOS procedure to build a list of physical address ranges and their corresponding memory types.
                Later, the kernel executes the machine_specific_memory_setup() function, which
                builds the physical addresses map (see Table 2-9 for an example). Of course, the kernel builds this table on the basis of the BIOS list, if this is available; otherwise the
                kernel builds the table following the conservative default setup: all page frames with
                numbers from 0x9f (LOWMEMSIZE()) to 0x100 (HIGH_MEMORY) are marked as reserved.


                so fo us we start allocating pages from the 1 MB.




                // starting from 64kb
                unsigned int free_mem_addr = 0x10000;

                // kernel malloc. 
                unsigned int kmalloc(unsigned int size, int alignment_4k_flag)
                {
                    if(alignment_4k_flag == 1 && (free_mem_addr & 0x00000FFF))
                    {
                        // we clear the parts that are not 
                        free_mem_addr &= 0xFFFFF000;

                        // We advance to the next 4k page
                        free_mem_addr += 0x1000;
                    }

                    unsigned int return_address = free_mem_addr;
                    free_mem_addr += size;
                    return return_address;
                }


think of my physical memory manager code keep on giving my paging code 4k memory.
Then my paging code converts it into a page.

         _______________                   ______________
        |               |                 |              |
        |   physical    |   4k memory     |              |
        |    memory     | --------------> |   my paging  | 
        |   manager     |                 |    code      |
        |               |                 |              |      
        |_______________|                 |______________|              
        


                // Converts a memory into page frame
                void add_page_frame(unsigned int memory_ptr, int is_kernel, int is_writeable)
                {
                    if (page->frame != 0)
                    {
                        return;
                    }
                    else
                    {
                        u32int idx = first_frame();
                        if (idx == (u32int)-1)
                        {
                            // PANIC! no free frames!!
                        }
                        set_frame(idx*0x1000);
                        page->present = 1;
                        page->rw = (is_writeable)?1:0;
                        page->user = (is_kernel)?0:1;
                        page->frame = idx;
                    }
                }




that is, bit #i in byte #n define the status of page #n*8+i

 malloc(8) -> malloc checks if it has any available block=no -> ask the virtual memory manager for some memory -> vmm finds a region and maps it using pages given by the pmm -> malloc returns a block.







https://forum.osdev.org/viewtopic.php?f=1&t=32370


This is how I think of memory management. It will take you some time to understand the whole concept.
There are 3 distinct memory management levels:
1.Physical memory. Here you can use a general purpose stack/bitmap allocator. 
    Your allocator needs to be able to return page aligned addresses (either 4 KB, 2 MB or 4 MB). 
    Here we use a concept called blocks, where each block is either 4 KB, 2 MB or 4 MB in size. 
    You will need a function (you will also need allocate_block(s), free_block(s)) that can reserve certain space from X to Y. 
    Make sure you use your multiboot memory map. If your memory map says something is used then make sure to reserve that area. 
    Everything that is free has to be marked as free. 
    Note: some memory is likely to be above 4 GB = not accessible in 32 bit protected mode, 
    you will need to either enable PAE or not use that memory (wasteful). 
    Remember, memory does not have to be contiguous and you PMM needs to be aware of this. 
    There can be holes as big as 1 GB between the memory map areas. 
    Making a huge bitmap/something else that can cover everything from 0x0 to 0xFFFFFFFFFFFFFFFF is not recommended, 
    better split your implementation per each free area, aka 1 free area = 1 separate bitmap. You dont have to use a bitmap, 
    there are other options as well, but it is the easiest one. Make sure you reserve and save those ACPI areas accordingly, 
    you can reclaim some of them after reallocating them. This memory manager is the one that you are supposed to reserve your kernel with. 
    Make sure you reserve your entire kernel from A to Z so things dont get overwritten. Consider making a physical address space manager. 
    (it could manage all of your devices and structures inside your memory used by the BIOS).

2.Virtual memory. This memory includes paging, everyones favorite subject. This is really straight forward. Enable paging, map all of its structures, use recursive page directory trick if you want. Make sure you map all of your PMM structures so you don't get unexpected triple faults. As stated before you can enable PAE, or not (if you don't care about wasting a few hundred MB or even up to 1 GB). Now, you will need to replicate the same exact functions as used in PMM but they will have to be virtual address space aware. You don't have to make an entirely new system. You could just use your PMM and map/unmap the addresses returned by it. Don't let paging scare you, once you figure it out you will like it. Intel manuals are always a great thing. You could also make functions such as page directory switching or page table cloning. When we talk about VMM we also talk about the higher half kernel. You can choose to use it or not. It has its pros and cons, I personally don't use it, yet. If your kernel is loaded at 1 MB you don't need to worry about first MB being used, you will need it for a Virtual 8086 monitor (if you decide to use it for graphics mode switching).
3.Dynamic memory. This is the part where we talk about malloc, free, calloc, realloc, etc... This is mostly user space related. 
This is a manager that people will use to create applications for your OS. 
(you primarily) Dynamic memory manager has to return a certain amount of bytes as requested. 
It is suggested to use an AVL tree for performance. It is "hard" to implement it at first, 
but it is the most efficient implementation (consider self balancing trees and red black ones). 
A linked list can be used for this and it is easy to work with, but it is really really slow and not recommended. 
You dont have to go through all this you can just port an existing allocator. Such as: dlmalloc, liballoc, tlfs, nedmalloc, ptmalloc, etc... I would personally suggest porting if you don't know what are you doing, but if you are familiar with binary tress go ahead. But what about the kernel, you didn't talk about that. Well you could make a simple allocator that would return 2, 4, 8, 16, 32, 64 bytes for kernel to use. You should use this allocator for kernel structures and "data" keeping. It is supposed to be on top of VMM. Ask VMM for couple of pages (pages in VMM) (blocks in PMM) and then parcel/slice/chop those pages into fragments/smaller pieces and that could be your pool (not the swimming one!).

I hope I was even the tinniest bit helpful.


Compare, for example, a bitmap. When you want to find a free page, each 32 bit long integer can contain a representation of 0x20000 bytes of physical RAM.


100000000000000000

2^16 = 64kb
2^17 = 128kb 

32 bit, each bit represents 4kb pages. so it can represent 128 kb.


Now, taking all this information into account, for a computer with 4 GB of RAM would you:
a) use bitmaps for 100% of RAM, even though it's slower and has more overhead
b) use free page stacks for 100% of RAM, even though it can't be used for contiguous allocations
c) manage some RAM with bitmaps (just enough to satisfy device drivers, plus some spare just in case) 
    and manage the rest of RAM with free page stacks, and get the best of both methods







Before I made my hybrid algorithm I made a generic bitmap algo, like the one in JamesMs tutorial, and made a benchmark tool for it.
Code:
273814 frames allocated in 1000 milliseconds (1069 MB)
387904 frames allocated in 2000 milliseconds (1515 MB)
472588 frames allocated in 3000 milliseconds (1846 MB)
542474 frames allocated in 4000 milliseconds (2119 MB)
603190 frames allocated in 5000 milliseconds (2356 MB)
670271 frames allocated in 6000 milliseconds (2618 MB)
724350 frames allocated in 7000 milliseconds (2829 MB)
774298 frames allocated in 8000 milliseconds (3024 MB)
821233 frames allocated in 9000 milliseconds (3207 MB)
864210 frames allocated in 10000 milliseconds (3375 MB)
901235 frames allocated in 11000 milliseconds (3520 MB)
941861 frames allocated in 12000 milliseconds (3679 MB)
979355 frames allocated in 13000 milliseconds (3825 MB)
1010964 frames allocated in 14000 milliseconds (3949 MB)


Basically it takes a whole 15 seconds to allocate 4gb of physical RAM. Also, the more ram that is used the slower allocations take to perform. This is compiled as a release build using Microsoft Visual C++ 2005.

edit:

A debug build of the same bitmap allocator:

Code:
138307 frames allocated in 1000 milliseconds (540 MB)
195893 frames allocated in 2000 milliseconds (765 MB)
239543 frames allocated in 3000 milliseconds (935 MB)
273832 frames allocated in 4000 milliseconds (1069 MB)
303816 frames allocated in 5000 milliseconds (1186 MB)
328441 frames allocated in 6000 milliseconds (1282 MB)
352115 frames allocated in 7000 milliseconds (1375 MB)
371408 frames allocated in 8000 milliseconds (1450 MB)
390836 frames allocated in 9000 milliseconds (1526 MB)
408839 frames allocated in 10000 milliseconds (1597 MB)
429535 frames allocated in 11000 milliseconds (1677 MB)
446240 frames allocated in 12000 milliseconds (1743 MB)
465661 frames allocated in 13000 milliseconds (1818 MB)
482927 frames allocated in 14000 milliseconds (1886 MB)


In the same amount of time it allocates less than half the amount of memory.



As this name suggests, this allocator uses a bit-map to keep track of the used and unused memory locations for its book-keeping purposes.





http://brokenthorn.com/Resources/OSDev26.html

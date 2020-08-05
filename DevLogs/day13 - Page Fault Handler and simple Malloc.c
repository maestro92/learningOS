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





########################################################################
############################### Malloc #################################
########################################################################
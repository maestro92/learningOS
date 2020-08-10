
unsigned int page_directory[1024] __attribute__((aligned(4096)));
unsigned int first_page_table[1024] __attribute__((aligned(4096)));

extern void load_paging_directory(unsigned int);
extern void enable_paging();


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

void init_paging()
{
	for(int i=0; i < 1024; i++)
	{
		page_directory[i] = 0x00000002;
	}
 

 	for(int i=0; i < 1024; i++)
 	{
 		first_page_table[i] = (i * 0x1000) | 3;
 	}

	// attributes: supervisor level, read/write, present
	page_directory[0] = ((unsigned int)first_page_table) | 3;

	load_paging_directory((unsigned int)&page_directory);
	enable_paging();
}
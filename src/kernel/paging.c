

#define ENTRIES_PER_TABLE 1024
#define ENTRIES_PER_DIR 1024

#define PAGE_DIRECTORY_INDEX(x) (((x) >> 22) & 0x3ff)
#define PAGE_TABLE_INDEX(x) (((x) >> 12) & 0x3ff)
#define PAGE_GET_PHYSICAL_ADDRESS(x) (*x & ~0xfff)


#define I86_PDE_PRESENT    1       //0000000000000000000000000000001
#define I86_PDE_WRITABLE       2       //0000000000000000000000000000010
#define I86_PDE_USER           4       //0000000000000000000000000000100
#define I86_PDE_PWT            8       //0000000000000000000000000001000
#define I86_PDE_PCD            0x10    //0000000000000000000000000010000
#define I86_PDE_ACCESSED       0x20    //0000000000000000000000000100000
#define I86_PDE_DIRTY          0x40    //0000000000000000000000001000000
#define I86_PDE_4MB            0x80    //0000000000000000000000010000000
#define I86_PDE_CPU_GLOBAL        0x100      //0000000000000000000000100000000
#define I86_PDE_LV4_GLOBAL        0x200      //0000000000000000000001000000000
#define I86_PDE_FRAME          0x7FFFF000  //1111111111111111111000000000000

#define I86_PTE_PRESENT       1       //0000000000000000000000000000001
#define I86_PTE_WRITABLE      2       //0000000000000000000000000000010
#define I86_PTE_USER          4       //0000000000000000000000000000100
#define I86_PTE_WRITETHOUGH   8       //0000000000000000000000000001000
#define I86_PTE_NOT_CACHEABLE 0x10    //0000000000000000000000000010000
#define I86_PTE_ACCESSED      0x20    //0000000000000000000000000100000
#define I86_PTE_DIRTY        0x40    //0000000000000000000000001000000
#define I86_PTE_PAT          0x80    //0000000000000000000000010000000
#define I86_PTE_CPU_GLOBAL   0x100      //0000000000000000000000100000000
#define I86_PTE_LV4_GLOBAL   0x200      //0000000000000000000001000000000
#define I86_PTE_FRAME        0x7FFFF000  //1111111111111111111000000000000


typedef unsigned int page_directory_entry;
typedef unsigned int page_table_entry;

struct PageTableStruct
{
   page_table_entry entries[ENTRIES_PER_TABLE];
} __attribute__((packed));
typedef struct PageTableStruct PageTable;

struct PageDirectoryStruct
{
   page_directory_entry entries[ENTRIES_PER_DIR];
}  __attribute__((packed));
typedef struct PageDirectoryStruct PageDirectory;


/*
unsigned int page_directory[1024] __attribute__((aligned(4096)));
unsigned int first_page_table[1024] __attribute__((aligned(4096)));
*/

extern void load_paging_directory(unsigned int);
extern void enable_paging();
extern void flush_tlb();



// page directory functions
void page_directory_entry_set_flags(page_directory_entry* entry, unsigned int attrib)
{
   *entry |= attrib;
}

void page_directory_entry_del_flags(page_directory_entry* entry, unsigned int attrib)
{
   *entry &= ~attrib;
}

void page_directory_entry_set_frame(page_directory_entry* entry, unsigned int page_address)
{
   *entry = (*entry & ~I86_PDE_FRAME) | page_address;
}



// page table functions
void page_table_entry_set_flags(page_table_entry* entry, unsigned int attrib)
{
   *entry |= attrib;
}

void page_table_entry_del_flags(page_table_entry* entry, unsigned int attrib)
{
   *entry &= ~attrib;
}

void page_table_entry_set_frame(page_table_entry* entry, unsigned int page_address)
{
   *entry = (*entry & ~I86_PTE_FRAME) | page_address;
}




/*
bool vmmgr_alloc_page(page_table_entry* entry)
{
   void* page_address = pmmgr_alloc_block();
   if(page_address == 0)
      return false;

   page_table_entry_set_frame(entry, (unsigned int)page_address);
   page_table_entry_set_attrib(entry, );

   return true;
}

void vmmgr_free_page(page_table_entry* entry)
{
   void* p = (void*)pt_entry_pfn (*e);
   if (p)
      pmmngr_free_block (p);
 
   page_table_entry_del_attrib (e, I86_PTE_PRESENT);
}
*/


// For the page directory




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

   // Allocate default page table 
   PageTable* table = (PageTable*) pmmgr_alloc_block();
   if(!table)
      return; 

   kprint_hex((unsigned int)table);
   kprint("\n");

   // Allocate 3gb page table 
   PageTable* table2 = (PageTable*) pmmgr_alloc_block();
   if(!table2)
      return; 

   kprint_hex((unsigned int)table2);
   kprint("\n");

   memory_set((unsigned char*)table, 0, sizeof(PageTable));
   memory_set((unsigned char*)table2, 0, sizeof(PageTable));


   // 1st 4mb are identity mapped
   // this way all the memory mapped areas
   // and the page tables are referred correction

   int frame = 0x0;
   int virt = 0x0;

   for(int i=0; i<1024; i++)
   {
      page_table_entry entry = 0;

      page_table_entry_set_flags(&entry, I86_PTE_PRESENT);
      page_table_entry_set_flags(&entry, I86_PTE_WRITABLE);
      page_table_entry_set_frame(&entry, frame);

      if(i < 10)
      {
      //   kprint_num(PAGE_TABLE_INDEX(virt));
         kprint_hex(entry);
         
         kprint("\n");
      }

      table->entries[PAGE_TABLE_INDEX(virt)] = entry;


      frame += 4096;
      virt += 4096;
      
   }


   // map kernel's current position to 3gb 
   // this was defined in bootsector-main.asm
   unsigned int kernel_pos = 0x1000;
   frame = kernel_pos;
   virt = 0xc0000000;
   for(int i=0; i<1024; i++)
   {
      page_table_entry entry = 0;

      page_table_entry_set_flags(&entry, I86_PTE_PRESENT);
      page_table_entry_set_flags(&entry, I86_PTE_WRITABLE);      
      page_table_entry_set_frame(&entry, frame);
   
      table2->entries[PAGE_TABLE_INDEX(virt)] = entry;

      frame += 4096;
      virt += 4096;
   }


   // add the two tables
   PageDirectory* page_directory = (PageDirectory*) pmmgr_alloc_block();

   kprint_hex((unsigned int)page_directory);
   kprint("\n");



   memory_set((unsigned char*)page_directory, 0, sizeof(PageDirectory));


   // table covers 0 to 4mb, so 
   page_directory_entry entry = 0;
   page_directory_entry_set_flags(&entry, I86_PDE_PRESENT);
   page_directory_entry_set_flags(&entry, I86_PDE_WRITABLE);
   page_directory_entry_set_frame(&entry, (unsigned int)table);
   page_directory->entries[PAGE_DIRECTORY_INDEX(0x00000000)] = entry;


   // table 2 maps kernel's position to 3gb, and 4mb from 3gb
   // so virt here is 0xc0000000
   page_directory_entry entry2 = 0;
   page_directory_entry_set_flags(&entry2, I86_PDE_PRESENT);
   page_directory_entry_set_flags(&entry2, I86_PDE_WRITABLE);
   page_directory_entry_set_frame(&entry2, (unsigned int)table2);
   page_directory->entries[PAGE_DIRECTORY_INDEX(0xc0000000)] = entry2;


   load_paging_directory((unsigned int)page_directory);
	enable_paging();
}
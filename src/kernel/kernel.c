
int kernel_main();


int start()
{
   kernel_main();
}

#define KERNEL_CODE_SEGMENT 0x08
#define KERNEL_DATA_SEGMENT 0x10

#include "../libc/util.c"
#include "../drivers/driver_util.c"
#include "../drivers/screen.c"
#include "../drivers/keyboard.c"
#include "idt.c"
#include "pmmgr.c"
#include "paging.c"












void test_print()
{
   clear_screen();
   kprint("Martin OS 2\n");
   kprint_hex(0xA);
   kprint("\n");   
   kprint_hex(0xFF);

   kprint_num(1234);
   kprint("\n\n");
 
   kprint_num(-1234);
   kprint_num(0);
   kprint("\n\n");
 //  kprint("Martin OS 2\n");   
/*
   char message[2000];; 
   char* cur = message;
   int size = 0;
   int lines = 26;
   int l = 0;
   int j = 0;

   int num = 0;

   for(l = 0; l<lines; l++)
   {
      for(j = 0; j<5; j++)
      {
         if(l == 23)
         {
            *cur = '0' + 10;
         }
         else
         {
            *cur = '0' + num;
         }
         cur++;
      }
      *cur = '\n';
      cur++;

      num++;
      if(num == 10)
      {
         num = 0;
      }
   }
   *cur = '\0';

   kprint(message);
   */
}



void test_paging()
{
   unsigned int *ptr = (unsigned int*)0x00000FFF;
   unsigned int do_page_fault = *ptr;

   unsigned int *ptr2 = (unsigned int*)0xB0000000;
   do_page_fault = *ptr2;
}



void test_physical_memory_mgr()
{
   unsigned int* p = (unsigned int*)pmmgr_alloc_block();
   if(p != 0)
   {
      kprint("allocated at ");
      kprint_hex((unsigned int)p);
      kprint("\n");
   }
   else
   {
      kprint("ERROR: allocation unsuccessful\n");
   }

   unsigned int* p2 = (unsigned int*)pmmgr_alloc_block();

   if(p2 != 0)
   {
      kprint("allocated at ");
      kprint_hex((unsigned int)p2);
      kprint("\n");
   }
   else
   {
      kprint("ERROR: allocation unsuccessful\n");
   }


   pmmgr_free_block(p);

   p = (unsigned int*)pmmgr_alloc_block();
   kprint("Unallocated p to free block 1. p is reallocated to ");
   kprint_hex((unsigned int)p);
   kprint("\n");

   pmmgr_free_block(p);
}


int kernel_main()
{
   test_print();

   initIDT();
   kprint("Init ID");

   test_interrupt();

   kprint("back here");

   asm volatile("sti");

   init_physical_memory_manager();
   test_physical_memory_mgr();


   init_paging();

   kprint("back here2");

 //  test_paging();
/*
   for(;;) 
   {
      asm("hlt");
   }
   */
//   for(;;);

}



void something()
{

}
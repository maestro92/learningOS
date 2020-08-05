
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
#include "paging.c"

void test_print()
{
   clear_screen();
   kprint("Martin OS 2\n");

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


int kernel_main()
{
   test_print();

   initIDT();
   kprint("Init ID");

   test_interrupt();

   kprint("back here");

   asm volatile("sti");

   init_paging();

   test_paging();
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
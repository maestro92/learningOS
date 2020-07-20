
int kernel_main();


int start()
{
   kernel_main();
}


#include "util.c"
#include "../drivers/driver_util.c"
#include "../drivers/screen.c"
#include "../drivers/keyboard.c"
#include "idt.c"

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




int kernel_main()
{
   test_print();

   initIDT();
   kprint("Init ID");

   test_interrupt();

   kprint("back here");

   asm volatile("sti");
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
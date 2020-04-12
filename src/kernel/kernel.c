// #include "../drivers/screen.c"

int kernel_main();

int _start()
{
   kernel_main();
}


int kernel_main()
{
   unsigned char* vga = (unsigned char*) 0xb8000;
   
   unsigned char* name = "Martin OS";
   char* cur = name;
   int index = 0;
   while (*cur != '\0')
   {
      vga[index] = *cur;
      vga[index+1] = 0x09;

      cur++;
      index += 2;
   }

  // print("Martin OS 2");
   for(;;);

   /*
   unsigned char* vga = (unsigned char*) 0xb8000;
   
   unsigned char* name = "Martin OS";
   char* cur = name;
   int index = 0;
   while (*cur != '\0')
   {
      vga[index] = *cur;
      vga[index+1] = 0x09;

      cur++;
      index += 2;
   }

   for(;;); //make sure our kernel never stops, with an infinite loop
   */
}

void something()
{

}
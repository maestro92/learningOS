
#define PHYSICAL_MEMORY_BLOCK_SIZE  4096

unsigned int* blocks_bitmap;
int bitmap_array_length;
unsigned int pmmgr_used_block;
unsigned int pmmgr_max_blocks;


int get_num_usable_block()
{
   return pmmgr_max_blocks - pmmgr_used_block;
}


int pmmgr_get_first_free_block_index()
{
   //! find the first free bit
   for (int i=0; i< bitmap_array_length; i++)
   {
      if (blocks_bitmap[i] != 0xffffffff)
      {
         for (int j=0; j<32; j++) 
         { 
            int bit = 1 << j;
            if (! (blocks_bitmap[i] & bit))
            {
               return i*32+j;
            }
         }
      }
   }

   return -1;
}



void pmmgr_set_block_free(unsigned int* bitmap, int block_index)
{
   bitmap[block_index / 32] &= ~ (1 << (block_index % 32));
}

void pmmgr_set_block_used(unsigned int* bitmap, int block_index)
{
   bitmap[block_index / 32] |= (1 << (block_index % 32));
}

void* pmmgr_alloc_block()
{
   if (get_num_usable_block <= 0)
      return 0;

   int block_index = pmmgr_get_first_free_block_index();

   if(block_index == -1)
      return 0;

   pmmgr_set_block_used(blocks_bitmap, block_index);


   unsigned int addr = block_index * PHYSICAL_MEMORY_BLOCK_SIZE;
   pmmgr_used_block++;
   return (void*)addr;
}

void* pmmgr_free_block(void* block_addr)
{
   int block_index = (unsigned int)block_addr / PHYSICAL_MEMORY_BLOCK_SIZE;

   pmmgr_set_block_free(blocks_bitmap, block_index);
   pmmgr_used_block--;
}

void pmmgr_set_memory_region_free(unsigned int* base, int num_bytes)
{
   int block_index = (unsigned int)base / PHYSICAL_MEMORY_BLOCK_SIZE;
   int num_blocks = num_bytes / PHYSICAL_MEMORY_BLOCK_SIZE;

   for(int i=0; i<num_blocks; i++)
   {
      pmmgr_set_block_free(blocks_bitmap, i);
      pmmgr_used_block--;
   }

   // block 0  (first page, 0 ~ 4k) should always be reserved
   pmmgr_set_block_used(blocks_bitmap, 0);
}

void pmmgr_set_memory_region_used(unsigned int* base, int num_bytes)
{
   int block_index = (unsigned int)base / PHYSICAL_MEMORY_BLOCK_SIZE;
   int num_blocks = num_bytes / PHYSICAL_MEMORY_BLOCK_SIZE;

   for(int i=0; i<num_blocks; i++)
   {
      pmmgr_set_block_used(blocks_bitmap, i);
      pmmgr_used_block++;
   }
}


void init_physical_memory_manager()
{
   // so we just have 4 pages.

   // assume we have 16 KB of ram   
   unsigned int ram_size = 0x4000;

   // we place the bitmap at 1MB
   pmmgr_max_blocks = ram_size / 0x1000;
   pmmgr_used_block = pmmgr_max_blocks;

   int bitmap_byte_size = pmmgr_max_blocks / 8;
   if(pmmgr_max_blocks % 8 > 0)
   {
      bitmap_byte_size++;
   }


   bitmap_array_length = pmmgr_max_blocks / 32;
   if(pmmgr_max_blocks % 32 > 0)
   {
      bitmap_array_length++;
   }



   // Start it at 1MB
   blocks_bitmap = (unsigned int*)0x100000; 

   // At first, we just set all the memory region to be used
   memory_set((unsigned char*)blocks_bitmap, 0xf, bitmap_byte_size);


   // we will just set the first 1MB to be used 
   unsigned int* base = 0;
   pmmgr_set_memory_region_used(0, 0x100000);


   // we will use 16 kb from 1MB + 4kb
   base = (unsigned int*)(0x100000 + 0x1000);
   pmmgr_set_memory_region_free(base, ram_size);
}

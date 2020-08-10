
#define low_16(address) (unsigned short)((address) & 0xFFFF)
#define high_16(address) (unsigned short)(((address) >> 16) & 0xFFFF)

/*
// starting from 64kb
unsigned int free_mem_addr = 0x10000;
*/

void memory_copy(unsigned char* src, unsigned char* dst, int numBytes)
{
    int i = 0;
    for(int i=0; i<numBytes; i++)
    {
        *(dst + i) = *(src + i);
    }
}                

void memory_set(unsigned char* dest, char val, int num_bytes)
{
    int i = 0;
    for(int i=0; i<num_bytes; i++)
    {
        dest[i] = val;
    }
}


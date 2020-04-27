
#define low_16(address) (unsigned short)((address) & 0xFFFF)
#define high_16(address) (unsigned short)(((address) >> 16) & 0xFFFF)

#define KERNEL_CODE_SEGMENT 0x08
#define KERNEL_DATA_SEGMENT 0x10

void memory_copy(unsigned char* src, unsigned char* dst, int numBytes)
{
    int i = 0;
    for(int i=0; i<numBytes; i++)
    {
        *(dst + i) = *(src + i);
    }
}                

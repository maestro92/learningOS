
#define low_16(address) (unsigned short)((address) & 0xFFFF)
#define high_16(address) (unsigned short)(((address) >> 16) & 0xFFFF)


void memory_copy(unsigned char* src, unsigned char* dst, int numBytes)
{
    int i = 0;
    for(int i=0; i<numBytes; i++)
    {
        *(dst + i) = *(src + i);
    }
}                

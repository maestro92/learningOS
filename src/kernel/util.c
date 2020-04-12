
void memory_copy(unsigned char* src, unsigned char* dst, int numBytes)
{
    int i = 0;
    for(int i=0; i<numBytes; i++)
    {
        *(dst + i) = *(src + i);
    }
}                

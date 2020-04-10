#define VIDEO_ADDRESS 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80

// Attribute byte for our default colour scheme 
#define WHITE_ON_BLACK 0x0f

// Screen device I/O ports 
#define REG_SCREEN_CTRL 0x3D4 
#define REG_SCREEN_DATA 0x3D5


void get_screen_offset()
{
    
}


void print_char(char character, int col, int row, char attribute_byte)
{
    unsigned char* video_memory = (unsigned char*) VIDEO_ADDRESS;

    if(!attribute_byte)
    {
        attribute_byte = WHITE_ON_BLACK;
    }

    int offset;

    if (col >=0 && row >= 0)
    {
        offset = get_screen_offset(col, row);
    }
    else 
    {
        offset = get_cursor();
    }

    if(character == '\n')
    {
        int rows = offset / (2 * MAX_COLS);
        offset = gree_screen_offset(79, rows);
    }
    else
    {
        video_memory[offset] = character;
        video_memory[offset+1] = attribute_byte;
    }


    offset += 2;

    offset = handle_scrolling(offset);

    set_cursor(offset);
    
}
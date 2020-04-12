#include "driver_util.c"

#define VIDEO_ADDRESS 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80

// Attribute byte for our default colour scheme 
#define WHITE_ON_BLACK 0x0f

// Screen device I/O ports 
#define REG_SCREEN_CTRL 0x3D4 
#define REG_SCREEN_DATA 0x3D5


// col is x, row is y
int get_screen_offset(int col, int row)
{
    return 2 * (row * MAX_COLS + col);
}

int get_offset_row(int offset)
{
    return offset % MAX_COLS;
}

int get_offset_col(int offset)
{
    return offset / MAX_COLS;
}

int set_cursor(int offset)
{
    offset /= 2;
    port_byte_out ( 0x3D4 , 0xE);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(offset >> 8));
    port_byte_out ( 0x3D4 , 0xF);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(offset & 0xff));
}

int get_cursor()
{
    port_byte_out ( REG_SCREEN_CTRL , 0xE);
    int offset = port_byte_in ( REG_SCREEN_DATA ) << 8;
    port_byte_out ( REG_SCREEN_CTRL , 0xF);
    offset += port_byte_in ( REG_SCREEN_DATA );
    // Since the cursor offset reported by the VGA hardware is the
    // number of characters , we multiply by two to convert it to
    // a character cell
    return offset * 2;
}

int is_valid_coord(int col, int row)
{
    return col >=0 && col < MAX_COLS && row >= 0 && row < MAX_ROWS;
}

// assume col and row is valid
void print_char(char character, int col, int row, char attribute_byte)
{
    unsigned char* video_memory = (unsigned char*) VIDEO_ADDRESS;

    if(!attribute_byte)
    {
        attribute_byte = WHITE_ON_BLACK;
    }

    int offset = get_screen_offset(col, row);

/*
    if (is_valid_coord(col, row))
    {
        offset = get_screen_offset(col, row);
    }
    else 
    {
        offset = get_cursor();
    }
*/
    if(character == '\n')
    {
        int rows = offset / (2 * MAX_COLS);

        // if its a new line character, we set the offset to the end of the current row,
        // so it will be advanced to the first col of the next row when we do offset+=2 at the 
        // end of this iteration.
        offset = get_screen_offset(MAX_COLS-1, rows);
    }
    else
    {
        video_memory[offset] = character;
        video_memory[offset+1] = attribute_byte;
    }


    offset += 2;

//    offset = handle_scrolling(offset);

    set_cursor(offset);
}

void print_at(char* string, int col, int row)
{
    int offset; 

    if(is_valid_coord(col, row))
    {
        offset = get_screen_offset(col, row);
        set_cursor(offset);
    }
    else
    {
        offset = get_cursor();
        row = get_offset_row(offset);
        col = get_offset_col(offset);
    }

    int i = 0;
    while(string[i] != '\0')
    {
        print_char(string[i], col, row, WHITE_ON_BLACK);
        i++;
    }
}


void print(char* string)
{
    print_at(string, -1, -1);
}



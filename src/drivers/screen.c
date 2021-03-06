#define VIDEO_ADDRESS 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80

// Attribute byte for our default colour scheme 
#define WHITE_ON_BLACK 0x0f
#define BLUE_ON_BLACK 0x09
#define MAGENTA_ON_BLACK 0x05

// Screen device I/O ports 
#define REG_SCREEN_CTRL 0x3D4 
#define REG_SCREEN_DATA 0x3D5


// col is x, row is y
int get_screen_index(int col, int row)
{
    return row * MAX_COLS + col;
}

int get_index_row(int index)
{
    return index / MAX_COLS;
}

int get_index_col(int index)
{
    return index % MAX_COLS;
}

int set_cursor_index(int index)
{
    port_byte_out ( REG_SCREEN_CTRL , 0xE);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(index >> 8));
    port_byte_out ( REG_SCREEN_CTRL , 0xF);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(index & 0xff));
}

int get_cursor_index()
{
    port_byte_out ( REG_SCREEN_CTRL , 0xE);
    int index = port_byte_in ( REG_SCREEN_DATA ) << 8;
    port_byte_out ( REG_SCREEN_CTRL , 0xF);
    index += port_byte_in ( REG_SCREEN_DATA );
    // Since the cursor offset reported by the VGA hardware is the
    // number of characters , we multiply by two to convert it to
    // a character cell
    return index;
}

int is_valid_coord(int col, int row)
{
    return col >=0 && col < MAX_COLS && row >= 0 && row < MAX_ROWS;
}

int is_valid_index(int index)
{
    return 0 <= index && index < (MAX_COLS * MAX_ROWS) - 1;
}

void handle_scrolling(int* index)
{

    int cur_index = *index;

    if(is_valid_index(cur_index))
    {
        // we r good
    }
    else
    {
        unsigned char* video_memory = (unsigned char*) VIDEO_ADDRESS;
        // move everything up a row 
        int src_index = MAX_COLS;
        int dst_index = 0;
        unsigned char* src = &(video_memory[src_index * 2]);
        unsigned char* dst = &(video_memory[dst_index * 2]);

        memory_copy(src, dst, (MAX_ROWS - 1) * MAX_COLS * 2);


        // clear last row 
        int i=0;
        for(i<0; i<MAX_COLS; i++)
        {
            int index = get_screen_index(i, MAX_ROWS - 1);
            video_memory[index * 2] = ' ';
            video_memory[index * 2 + 1] = WHITE_ON_BLACK;   
        }
        *index = get_screen_index(0, MAX_ROWS-1);
    }
}

// assume col and row is valid
void print_char(char character, int col, int row, char attribute_byte)
{
    unsigned char* video_memory = (unsigned char*) VIDEO_ADDRESS;

    if(!attribute_byte)
    {
        attribute_byte = MAGENTA_ON_BLACK;
    }

    if(character == ':')
    {
        int a = 1;
    }

    int index = 0;

    if (is_valid_coord(col, row))
    {
        index = get_screen_index(col, row);
    }
    else 
    {
        index = get_cursor_index();
    }

    if(character == '\n')
    {
        int rows = get_index_row(index);

        // if its a new line character, we set the offset to the end of the current row,
        // so it will be advanced to the first col of the next row when we do offset+=2 at the 
        // end of this iteration.
        index = get_screen_index(MAX_COLS-1, rows);
    }
    else
    {
        video_memory[index * 2] = character;
        video_memory[index * 2 + 1] = attribute_byte;
    }


    index++;

    handle_scrolling(&index);

    set_cursor_index(index);
}

void print_at(char* string, int col, int row)
{
    int index; 

    if(is_valid_coord(col, row))
    {
        index = get_screen_index(col, row);
        set_cursor_index(index);
    }

    int i = 0;
    while(string[i] != '\0')
    {
        print_char(string[i], col, row, MAGENTA_ON_BLACK);
        i++;
    }
}


void kprint(char* string)
{
    print_at(string, -1, -1);
}


void kprint_char(char c)
{
    char tmp[2];
    tmp[0] = c;
    tmp[1] = '\0';
    kprint(tmp);
}

void kprint_num(int num)
{    
    int sign = 0;
    int cur_num = num;

    if(num >= 0)
    {
        sign = 1;
    }
    else
    {
        sign = -1;
        cur_num = -cur_num;
    }

    int length = 0;
    
    while(cur_num > 0)
    {
        cur_num /= 10;
        length++;        
    }


    if(sign == -1)
    {
        length++;
    }
    char tmp[length + 1];
    tmp[length] = '\0';

    // for debugging
    for(int i=0; i<length; i++)
    {
        tmp[i] = ' ';
    }



    int i = 0;
    cur_num = num;
    if(sign == -1)
    {
        tmp[0] = '-';
        i = 1;
        cur_num = -cur_num;
    }

    i = length - 1;
    while(cur_num > 0)
    {
        int remainder = cur_num % 10;
        cur_num /= 10;
        tmp[i--] = '0' + remainder;
    }
    kprint(tmp);
}



void kprint_hex(unsigned int n)
{
    int base = 16;
    
    if (n == 0)
    {
        kprint_char ('0');
        return;
    }

    char hex_numbers[16] = "0123456789ABCDEF";

    char output[11];
    output[10] = '\0';
    // for debugging
    for(int i=0; i<10; i++)
    {
        output[i] = ' ';
    }


    output[0] = '0';
    output[1] = 'x';

    int i = 9;
    int res = 0;
    while (n > 0)
    {
        res = n % 16;
        output[i] = hex_numbers[res];
        i--;
        n = n / 16;
    }

    while(i >= 2)
    {
        output[i] = '0';
        i--;
    }


    kprint(output);
}


void clear_screen()
{
    int r = 0;
    int c = 0;

    for(int r = 0; r < MAX_ROWS; r++)
    {
        for(int c = 0; c < MAX_COLS; c++)
        {
            print_char(' ', c, r, WHITE_ON_BLACK);
        }
    }
    set_cursor_index(get_screen_index(0, 0));
}
#define ENTER_KEY_CODE 0x1C

#if 0
unsigned char keyboard_map[128] = 
{
  0,  27, /* Esc */
  '1', '2', '3', '4', '5', '6', '7', '8', 
  '9', '0', '-', '=', 
  '\b', /* Backspace */
  '\t', /* Tab */
  'q', 'w', 'e', 'r',   
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', 
  '\n', /* Enter key */
    0,  /* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 39 */
 '\'', '`',   
    0,        /* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',            /* 49 */
  'm', ',', '.', '/',   0,              /* Right shift */
  '*',
    0,  /* Alt */
  ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
  '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
  '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};
#endif

unsigned char keyboard_map[128];

void init_keyboard_driver()
{

    #if 1
    unsigned char keyboard_map_temp[128] = {
      0,  27, /* Esc */
      '1', '2', '3', '4', '5', '6', '7', '8', 
      '9', '0', '-', '=', 
      '\b', /* Backspace */
      '\t', /* Tab */
      'q', 'w', 'e', 'r',   
      't', 'y', 'u', 'i', 'o', 'p', '[', ']', 
      '\n', /* Enter key */
        0,  /* 29   - Control */
      'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 39 */
     '\'', '`',   
        0,        /* Left shift */
     '\\', 'z', 'x', 'c', 'v', 'b', 'n',            /* 49 */
      'm', ',', '.', '/',   0,              /* Right shift */
      '*',
        0,  /* Alt */
      ' ',  /* Space bar */
        0,  /* Caps lock */
        0,  /* 59 - F1 key ... > */
        0,   0,   0,   0,   0,   0,   0,   0,
        0,  /* < ... F10 */
        0,  /* 69 - Num lock*/
        0,  /* Scroll Lock */
        0,  /* Home key */
        0,  /* Up Arrow */
        0,  /* Page Up */
      '-',
        0,  /* Left Arrow */
        0,
        0,  /* Right Arrow */
      '+',
        0,  /* 79 - End key*/
        0,  /* Down Arrow */
        0,  /* Page Down */
        0,  /* Insert Key */
        0,  /* Delete Key */
        0,   0,   0,
        0,  /* F11 Key */
        0,  /* F12 Key */
        0,  /* All other keys are undefined */
    };
    #endif

    memory_copy(keyboard_map_temp, keyboard_map, 128);
}

void keyboard_handler()
{
    unsigned char scan_code = port_byte_in(0x60);
    unsigned char ascii = keyboard_map[scan_code];
    char str[2] = {ascii, '\0'};
    kprint(str);
}

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

//! shift, alt, and ctrl keys current state
unsigned char _shift, _alt, _ctrl;


enum ModfierScanCode {
  KEY_LSHIFT            = 0x2a,
  KEY_LCTRL             = 0x1d,
  KEY_LALT              = 0x38,

  KEY_RSHIFT            = 0x36,
//  KEY_RCTRL             = 0x4007,
//  KEY_RALT              = 0x4008,
};

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
    _shift = _alt = _ctrl = 0;
    memory_copy(keyboard_map_temp, keyboard_map, 128);
}

int is_modifier_key(unsigned char scan_code)
{
    return scan_code == KEY_LCTRL || 
         scan_code == KEY_LSHIFT || 
         scan_code == KEY_RSHIFT || 
         scan_code == KEY_LALT;
/*
  return scan_code == KEY_LCTRL || 
         scan_code == KEY_RCTRL || 
         scan_code == KEY_LSHIFT || 
         scan_code == KEY_RSHIFT || 
         scan_code == KEY_LALT || 
         scan_code == KEY_RALT;
*/
}


unsigned char apply_modifier_key(unsigned ascii)
{
  if(_shift)
  {
    if('a' <= ascii && ascii <= 'z')
    {
      ascii -= 32;
    }
    
    if('0' <= ascii && ascii <= '9')
    {
      switch(ascii)
      {
        case '0':
          ascii = ')';
          break;  
        case '1':
          ascii = '!';
          break;  
        case '2':
          ascii = '@';
          break;  
        case '3':
          ascii = '#';
          break;  
        case '4':
          ascii = '$';
          break;  
        case '5':
          ascii = '%';
          break;  
        case '6':
          ascii = '^';
          break;  
        case '7':
          ascii = '&';
          break;  
        case '8':
          ascii = '*';
          break;  
        case '9':
          ascii = '(';
          break;                          
      }
    }
  }

  return ascii;
}


void keyboard_handler()
{
    unsigned char scan_code = port_byte_in(0x60);

 //   kprint_hex(scan_code);
 //   kprint("\n");

    // Again, we either have a make code or a breakcode
    // test if this is a break code

    static int _extended = 0;

    if(scan_code == 0xE0 || scan_code == 0xE1)
    {
      _extended = 1;
    }
    else 
    {
      if(scan_code & 0x80)
      {
        scan_code -= 0x80;
        
        if(is_modifier_key(scan_code))
        {
          switch(scan_code)
          {
            case KEY_LCTRL:
              _ctrl = 0;
              break;

            case KEY_LSHIFT:
            case KEY_RSHIFT:
              _shift = 0;
              break;

            case KEY_LALT:
              _alt = 0;
              break;
          }
        }
      }
      else
      {
        // make code
        if(is_modifier_key(scan_code))
        {      
          switch(scan_code)
          {
            case KEY_LCTRL:
              _ctrl = 1;
              break;

            case KEY_LSHIFT:
            case KEY_RSHIFT:
              _shift = 1;
              break;

            case KEY_LALT:
              _alt = 1;
              break;
          }
        }
        else 
        {
          unsigned char ascii = keyboard_map[scan_code];
          ascii = apply_modifier_key(ascii);
          char str[2] = {ascii, '\0'};
          kprint(str);
        }
      }

      _extended = 0;
    }
}

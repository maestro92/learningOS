Chapter 6
https://www.cs.bham.ac.uk/~exr/lectures/opsys/10_11/lectures/os-dev.pdf

So with our current VGA port, we are writing into the VGA text memory.
Using the graph from day4


                 ___________________    <--------- 0x00000000 
                |                   |
                |                   |
                |                   |
                |___________________|   <-------- 0x00007C00 (31 kb)
                |    bootsector     |
                |                   |   
                |___________________|  
                |                   |
                |                   |
                |                   |   
                |                   |
                |                   |   
                |                   |
                |___________________|   <--------- 0x000B8000 (736 kb)
                |                   |
                |   CGA Graphics    |
                |   CGA, EGA &      |
                |   VGA Text        |
                |___________________|   <--------- 0x000BC000 (752 kb)
                |                   |
                |                   |   
                |                   |
                |                   |               
                |                   |
                |                   |
                |                   |
                |                   |               
                |                   |
                |                   |
                |                   |
                |                   |
                |                   |
                |                   |
                |                   |   <--------- 0x100000 (1MB)

So we want to understand how does CPU interact with hardware a little bit better. 

Taking the now-popular TFT monitor as an example, 


So the CPU and hardare talks to each other through I/O Bus.
https://www.karbosguide.com/hardware/module2c1.html

Visually it looks like this 

         _______________         _______________                         _______________
        |               |       |               |                       |               |
        |               |       |               |                       |               |
        |               |       |               |                       |               |
        |               |-------|               |-----------------------|               |
        |   CPU         |       |     RAM       |                       |    I/O        |
        |               |       |               |       Bus             |   Units       |
        |               |       |               |                       |               |
        |               |-------|               |-----------------------|               |
        |               |       |               |                       |               |
        |               |       |               |                       |               |
        |_______________|       |_______________|                       |_______________|

The bus controller is responsible for relaying, at a compatible rate, the instructions to a particular device_s controller. 

So hardware controller chips have several registers that can be read, written or both by the CPU, and it is the state of these 
registers that tell the hardware what to do. 

The question is, how do we tell our devices what to do programmtically? how do we communicate with our hardware devices?
In Intel architecture systems, the registers of hardware device controllers are mapped into an I/O address space. Then 
varients of the I/O instructions "in" and "out" are used to read and write data to the I/O addresses that are mapped to specific controller 
registers. 

What we mean by I/O address space is: recall this map from day 4
https://manybutfinite.com/post/motherboard-chipsets-memory-map/

we reserver a portion of the physical ram for devices

also referencing the Wiki page again,
https://en.wikipedia.org/wiki/Memory-mapped_I/O

it says:

        "The memory and registers of the I/O devices are mapped (associated with) address values. So when 
        and address is accessed by the CPU, it may refer to the a portion of physical RAM, or it can instead refer to memory 
        of the I/O device. 

        Each I/O device monitors the CPU's address bus and responds to any CPU access of an address assigned to that device, connecting the data bus 
        to the desired dvice's hardware register.
        "

Essentially, CPU modifies address values, which modifies register, which controls the hardware.

From the CPU_s perspective, an I/O device appears as a set of special-purpose registers, of three general types:
-   Control register
-   Status register
-   Data register

a logical circuit that contains these registers is called the device controller, and the software communicates with the controller is called a device driver

http://www.cs.uwm.edu/classes/cs315/Bacon/Lecture/HTML/ch14s03.html

                            +-------------------+           +-----------+
                            | Device controller |           |           |   
        +-------+           |                   |<--------->|  Device   |
        |       |---------->| Control register  |           |           |
        |  CPU  |<----------| Status register   |           |           |
        |       |<--------->| Data register     |           |           |
        +-------+           |                   |           |           |
                            +-------------------+           +-----------+



For example, flobby disk controller has its DOR register mapped to I/O address 0x3f2

https://en.wikipedia.org/wiki/Floppy-disk_controller
and in terms of assembly code, you can swtich on the motor of the first drive with the following instructions 


                mov dx , 0 x3f2         ; Must use DX to store port address
                in al , dx              ; Read contents of port ( i.e. DOR ) to AL
                or al , 00001000 b      ; Switch on the motor bit
                out dx , al             ; Update DOR of the device.





###########################################################
################# Direct Memory Access ####################
###########################################################

So there is another concept we need to introduce since we are on the topic of I/O devices, which is 
the Direct Memory Access Controller. Since port I/O invovles reading or writing individual bytes or words,
the transfer or large amouts of data between a disk device and memory could potentially take up a great 
deal of CPU time. So the CPU usually pass over this tedious task to the direct memory (DMA) controller.

https://en.wikipedia.org/wiki/Direct_memory_access

Quoting the wiki page:

        
        "Direct memory access is a feature of computer systems that allows certain hardware subsystems to access main system memory 
        (random-access memory), independent of the CPU. 

        Without DMA, when the CPU is using programmed input/output, it is typically fully occupied for the entire duration of the read or write operation, 
        and is thus unavailable to perform other work. With DMA, the CPU first initiates the transfer, 
        then it does other operations while the transfer is in progress, and it finally receives an interrupt from the DMA controller (DMAC) 
        when the operation is done. This feature is useful at any time that the CPU cannot keep up with the rate of data transfer, 
        or when the CPU needs to perform work while waiting for a relatively slow I/O data transfer."




###########################################################################
############################## Screen Driver ##############################
###########################################################################

So now we want to start writing our first device driver. We want to write a Screen Driver. Essentially, its an abstraction of the screen 
that will allow us to write print("Hello"); and perhaps clear_screen(); It will also allow us to easily substitute one display driver 
for another at a later date, perhaps if a certain computer could not support the colour VGA text mode that we currently assume. 

So we will write thhis screen.c, and we will put it in the drivers folder.

Another thing is that we organize all of our files into 3 folders now:

all the boot_sector related code to the boot folder, 
kernel.c in the kernel folder,
and the drivers folder that we just created 

        /boot 
        /drivers
        /kernel



So this in terms of messing with the VGA registers, this is where we have to look at the specs. 

The full offical specs on vga register from intel is here
https://01.org/sites/default/files/documentation/ilk_ihd_os_vol3_part1r2_0.pdf

chapter 2.6:
        
        "The CRT controller registers are accessed by writing the index of the desired register into the CRT Controller
        Index Register at I/O address 3B4h or 3D4h, depending on whether the graphics system is configured for
        MDA or CGA emulation. The desired register is then accessed through the data port for the CRT controller
        registers located at I/O address 3B5h or 3D5h, again depending upon the choice of MDA or CGA emulation as
        per MSR[0]. For memory mapped accesses, the Index register is at 3B4h (MDA mode) or 3D3h (CGA mode)
        and the data port is accessed at 3B5h (MDA mode) or 3D5h (CGA mode)."

Essentially, the Control Index register is at 3B4h (MDA mode) or 3D4h (CGA mode)
and the data port is accessed at 3B5h (MDA mode) or 3D5h (CGA mode). I believe the "3D3h (CGA mode)" is a typo?

MDA is the monochrome mode, CGA is color mode


also note what it says

        "The CRT controller registers are accessed by writing the index of the desired register into the CRT Controller
        Index Register at I/O address 3B4h or 3D4h,

        The desired register is then accessed through the data port for the CRT controller
        registers located at I/O address 3B5h or 3D5h
        "

[if you scroll down in the intel spec, you can see a list of detailed description of what each register does].

the idea is that you have to write to the Control register first, then access the register you want through the data port.
We will see what that means in the code below:

So lets take the example of the 
taking the example of the "get_cursor()" function in the os-dev.pdf 

we will look at how port_byte_out(); and port_byte_in() works later. Now just pretend that these two functions read and write to a port

                int get_cursor()
                {
                    port_byte_out ( REG_SCREEN_CTRL , 14);
                    int offset = port_byte_in ( REG_SCREEN_DATA ) << 8;
                    port_byte_out ( REG_SCREEN_CTRL , 15);
                    offset += port_byte_in ( REG_SCREEN_DATA );
                    // Since the cursor offset reported by the VGA hardware is the
                    // number of characters , we multiply by two to convert it to
                    // a character cell

                    return offset * 2;
                }

here 
REG_SCREEN_CTRL is 0x3D4
REG_SCREEN_DATA is 0x3D5

14 in hex is 0xE
15 in hex is 0xF

so effectively its doing 

                int get_cursor()
                {
                    port_byte_out ( 0x3D4 , 0xE);
                    int offset = port_byte_in ( 0x3D5 ) << 8;
                    port_byte_out ( 0x3D4 , 0xF);
                    offset += port_byte_in ( 0x3D5 );
                    // Since the cursor offset reported by the VGA hardware is the
                    // number of characters , we multiply by two to convert it to
                    // a character cell

                    return offset * 2;
                }



if you look at page 64 of the intel spec I linked above, it says 

        2.6.16 CR0E ⎯Text Cursor Location High Register
        I/O (and Memory Offset) Address: 3B5h/3D5h (index=0Eh)
        Default: Undefined
        Attributes: Read/Write 

        Text Cursor Location Bits [15:8]. This field provides the 8 most significant bits of a 16-bit value that
        specifies the address offset from the beginning of the frame buffer at which the text cursor is located. Bit
        7:0 of the Text Cursor Location Low Register (CR0F) provide the 8 least significant bits.



and then 

        2.6.17 CR0F ⎯Text Cursor Location Low Register
        I/O (and Memory Offset) Address: 3B5h/3D5h (index=0Fh)
        Default: Undefined
        Attributes: Read/Write

        7:0 Text Cursor Location Bits [7:0]. This field provides the 8 least significant bits of a 16-bit value that
        specifies the address offset from the beginning of the frame buffer at which the text cursor is located. Bits
        7:0 of the Text Cursor Location High Register (CR0D) provide the 8 most significant bits.

so these two registers tells you the 16-bit value that specifies the address offset of the text cursor.
so the first register, we push it up 8 bits, and then add it with the lower 8 bits which we read from the 2nd register.

but before we access these two register, we have to write to the control register first with the index 


                int get_cursor()
                {
    ----------->    port_byte_out ( 0x3D4 , 0xE);
                    int offset = port_byte_in ( 0x3D5 ) << 8;
    ----------->    port_byte_out ( 0x3D4 , 0xF);
                    offset += port_byte_in ( 0x3D5 );
                    // Since the cursor offset reported by the VGA hardware is the
                    // number of characters , we multiply by two to convert it to
                    // a character cell

                    return offset * 2;
                }

hence these 4 lines.


more relevant links can be found here 

https://wiki.osdev.org/Text_Mode_Cursor#Get_Cursor_Position
https://wiki.osdev.org/VGA_Hardware


so for the set_cursor(); its similar. Instead of reading the data, we are writing the data to the registers 


                int set_cursor(int offset)
                {
                    offset /= 2;
                    port_byte_out ( 0x3D4 , 0xE);
                    port_byte_out(REG_SCREEN_DATA, (unsigned char)(offset >> 8));
                    port_byte_out ( 0x3D4 , 0xF);
                    port_byte_out(REG_SCREEN_DATA, (unsigned char)(offset & 0xff));
                }




Now we get the gist of programming VGA registers, now lets look at our device driver code. 

so essentially we are trying to write a print function. So our main function is:
                
                screen.c

                int print_char(char c, int col, int row, char attr) 
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

                        // if its a new line character, we set the offset to the end of the current row,
                        // so it will be advanced to the first col of the next row when we do offset+=2 at the 
                        // end of this iteration.
                        offset = gree_screen_offset(MAX_COLS-1, rows);
                    }
                    else
                    {
                        video_memory[offset] = character;
                        video_memory[offset+1] = attribute_byte;
                    }


                    offset += 2;

                    set_cursor(offset);
                }


obviously, if we want to print a string, we just do a for loop calling print_char

                
                screen.c 

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



Everything should be pretty straight forward



####################################################################################
###################### port_byte_out(); and port_byte_in(); ########################
####################################################################################

Now lets take a look at how these functions work 

                unsigned char port_byte_in(unsigned short port)
                {
                    unsigned char result;
                    __asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
                    return result;
                }
                
                void port_byte_in(unsigned short port, unsigned char data)
                {
                    __asm__ ("out %% al , %% dx " : " a" ( data ), "d" ( port ));
                }

so a few things:

__asm__ is inline assembly in C code. The __asm__ keyword allows you to embed assembler instruction within C code. 


        "GCC provides two forms of inline asm statements. A basic asm statement is one with no operands, 
        while an extended asm statement includes one or more operands."

So there is basic assembly which is like 
                
                __asm__( your assembly instruction )

for example:
                __asm__("int $3")
https://gcc.gnu.org/onlinedocs/gcc/Basic-Asm.html#Basic-Asm



and there is extended assembly instruction, which looks like
                
                __asm__(  AssemblerTemplate : OutputOperands  )
                

https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html




So the code here written in the port_byte_in(); and port_byte_out(); function are all extended assembly instructions 
Lets so now lets take a closer look:

                "in %%dx, %%al" : "=a" (result) : "d" (port)
                "out %%al , %% dx" :" a" ( data ), "d" ( port )

this is essentially calling the "in" and "out" assembly instructions. 
https://c9x.me/x86/html/file_module_x86_id_139.html
https://c9x.me/x86/html/file_module_x86_id_222.html

you can see the comments says

        "Input byte from I/O port in DX into AL."


-   "in %%dx, %%al"
% is used to denote registers, so if you want your assembly instruction to actually be 

                in %dx, $al

you actually need:
                
                "in %%dx, %%al"

The GCC specs says 
https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html

        "
        ‘%%’
        Outputs a single ‘%’ into the assembler code.
        "

and I guess thats why you do this.


With extended asm you can read and write C variables from assembler and perform jumps from assembler code to C 
labels. Extended asm syntax uses colons (':') to delimit the operand parameters after the asembler template. 
https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html#InputOperands
https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html#OutputOperands

so in an asm statement, you can attach zero or more output operands indicating the names of the C variables 
modified by the assembler code. 


each oeprand has the format of 
                
                [ [asmSymbolicName] ] constraint (cvariablename)

the "constraints" essentially describes constraints on the cvariablename.

-   "=a" (result)
"=a" is the contraint. Output constarints must begin with either '='
'a' stands for types of "address register"
result is the cvariablename
https://gcc.gnu.org/onlinedocs/gcc/Simple-Constraints.html#Simple-Constraints

so what we are doing is putting AL register in variable RESULT when finished


-   "d" (port)
'd' stands for type of "data register"

port is the cvariablename
https://gcc.gnu.org/onlinedocs/gcc/Using-Assembly-Language-with-C.html

load EDX with port










################################################################
###################### Handle Scrolling ########################
################################################################

now we want to handle the case where when we print to the bottom of our screen, we need to scroll our screen.
so we add a handle scroling function







        "The VGA has over 300 internal registers, , "






we also create a utility function. It contains reading and writing bytes and words from a port.
Since these will be used by most device drivers, we put them in a utility file. 
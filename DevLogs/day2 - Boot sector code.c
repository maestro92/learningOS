
So the tutorial I am following is: 
https://www.cs.bham.ac.uk/~exr/lectures/opsys/10_11/lectures/os-dev.pdf
https://github.com/cfenollosa/os-tutorial


##########################################################################################
################################## bootloader ############################################
##########################################################################################

a bit of background of how a bootloader works.

so imagine your mother board looks like this:

and on this motherboard, there is a chip that contains a bit of code. That piece of code is called BIOS.
The BIOS is stored in ROM (read only memory);, and it is accessed for the first time by the CPU. 
BIOS is Basic Input Output System. Essentially, the BIOS code is baked into the motherboard of your PC. 

(A side note: the CPU can only receive instructions directly from either the BIOS or the RAM);
         ___________________________
        |      motherboard          |
        |                           |
        |     _______               |
        |    |       |              |
        |    | ROM   |              |
        |    |_______|              |
        |                           |
        |                           |
        |                           |
        |                           |
        |                           |
        |                           |
        |___________________________|

when you start your computer. BIOS is the first software that runs. It identifies your computer_s hardware, 
configures it, test it, and connects it to your operating system for further instructions. This is called the bootprocess.

BIOS is operating system independent. its main purpose is to test the hardware. 

The BIOS finds a target device to boot from that contains a master boot record(MBR); usually from Disk Drive, SD Card Reader, CD/DVD Rom or Hard Drive;
When you hit F2 or F12 to change the boot sequence, this is what its happening. 

The BIOS then transfer control to MBR. It loads and executes the initial bootstrap program from the master boot record. 
It is located in the first sector of the bootable disk. 

so imagine your hard disk (https://www.javatpoint.com/os-master-boot-record);

the very first sector of any hard disk is the MBR. 

         _____________________________________________________________
        |  MBR     |  |  |  |             |             |             |                                                        
        |__________|__|__|__|_____________|_____________|_____________|

        <-----------------------  Entire Disk ------------------------>



of course you also have the Partition Table and Disk Parititons

                 Partition Table         Disk Partitions
                    |  |  |            /        |        \
                    |  |  |           /         |         \
                    v  v  v          v          v          v
         _____________________________________________________________
        |  MBR     |  |  |  |             |             |             |                                                        
        |__________|__|__|__|_____________|_____________|_____________|

        <-----------------------  Entire Disk ------------------------>


the important thing is that MBR is at the first sector of the hard disk. It is 512 bytes in size. 
The first 512 bytes is the MBR

So the MBR also contains the MBR bootstrap program, as well as info on the partition table. 
https://wiki.osdev.org/MBR_(x86)

You can also look at the format of the x86 MBR
in the section "MBR Format", we want to point out two important parts 

Offset  Size    Description
0x000   440     MBR Bootstrap (flat binary executable code)
0x1FE   2       (0x55, 0xAA) "Valid bootsector" signature bytes


so there is some executable code at the very 440 bytes. That is the bootstrap program.

The 2nd thing is the magic numbers at the very end. The (legacy) BIOS check bootable devices for a magic number in these 512 bytes. 
It checks if byte 510 is 0x55 and byte 511 is 0xAA or (byte 511 is 0x55 or 512 is 0xAA, depending on how you phrase it); 
Essentially the last two bytes is 0x55 0xAA. If you consider that x86 is little-endian, the last number is 0xAA55 


knowing this, we have a very simple boot sector will look like:
            

                e9 fd ff 00 00 00 00 00 00 00 00 00 00 00 00 00
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
                [ 29 more lines with sixteen zero-bytes each ]
                00 00 00 00 00 00 00 00 00 00 00 00 00 00 55 aa

                                                            ^
                                                            |

The first 3 bytes perform an infinite jump. That is our Boottrap code 

so for our first boot sector code, it looks like:
            
                // (e9 fd ff)
                loop:   
                    jmp loop

                times 510-($-$$) db 0
                dw 0xaa55

note that this is written in NASM
[https://github.com/cfenollosa/os-tutorial/tree/master/01-bootsector-barebones]



Now to run this our bootloader code, we can just run the line 

                nasm -f bin boot_sect_simple.asm -o boot_sect_simple.bin


the -f bin option is used to instruct nasm to produce raw machine code, rather than a code 
package that has additional meta information for linking in other routines that we would expect to use 
when programming in a more typical Operating system enviornment 



and then we run it in qemu 
                
                qemu-system-x86_64 boot_sect_simple.bin


so to understand what is going on, so if you look at the qemu command 
                
                qemu-system-x86_64 [disk_image]

disk_image is essentially a binary map of the hard disk. so when we compile our boot sector code as a binary code,
we give it to qemu, and qemu will interpret it as a hard disk.






https://neosmart.net/wiki/mbr-boot-process/
if you think about it, 440 bytes is very little, so usually what happens is that you have to look up 
another file from the disk and load it to perform the actual boot process. So oftentimes 
the job of the bootstrap code segment in the MBR is pretty simple: look up the active partition from the partition 
table, and load that code into the memory for the execution by the CPU as the next link in the boot chain. 

so it goes from the BIOS to the bootstrap code in the MBR, then from the MBR to the bootstrap code in the partition bootsector, 
and from there to the executable boot files on the active partition, the actual logic involved in determining which
 operating system to load, where to load it from, which parameters/options to pass on to it, 
 and completing any interactions with the user that might be available, the actual process of starting the operating system begins.

So the actual bootloader files on the disk forms the final parts of the boot loading process. When people talk about 
bootloaders and boot files, they are often referring to this final, critical step of the boot process. 


So almost all bootlaoders separate the actual, executable bootloader from the configuration file or database 
that contains information about the OS to load. All of the bootloaders mentioned below have support for loading 
multiple OS, a process known as multi-booting. 

There are many bootloaders out there, each os has its own bootloader, specifically designed to read its file system 
and locate the kernel that needs to be loaded for OS to run. Each of the popular OS has its own default bootloader.
Windows NT, 2000, and XPas well as Windows Server 2000, and WIndows Server 2003 use the NTLDR bootloader.
WIndows Vista introduced the BOOTMGR bootloader, currently used by Windows Vista, 7, 8, 10. For Linux, 
GRUB is the most popular bootloader for Linux, though it can boot numerous other OSes as well. 


GRUB_s confugration file containing a whitespace-formatted list of OS was often called menu.lst or grub.lst,
and found under the /boot/ or /boot/grub/ directory

now there is GRUB2l which is confusingly called GRUB. The previous GRUB is now legacy GRUB. 
the actual bootloader file for GRUB2 is usually called core.img. 





https://help.ubuntu.com/community/Grub2/Installing
if you actually go to the Grub2 site, it says: 

        " GRUB 1.99 is included on Ubuntu releases 11.04 (Natty Narwhal) and later.

        The GRUB 2 bootloader is included on all currently-supported versions of the Ubuntu family. 
        GRUB 2 can accomodate traditional computer firmware such as BIOS as well as the newer EFI/UEFI standards. 
        It is compatible with MBR, GPT and other partitioning tables."

so essentially when you download a OS, and you burn it onto a USB stick, GRUB is included. 








https://askubuntu.com/questions/347203/what-exactly-is-grub



TL:DR, there are a few stages 

System startup              BIOS

    |
    |
    v 
Stage 1 bootloader          Master Boot Record 

    |
    |
    v
stage 2 bootloader          GRUB 

    |
    |
    v
  Kernel                    Linux

    |
    |
    v
   init                     User space


links:
https://en.wikipedia.org/wiki/GNU_GRUB
https://www.youtube.com/watch?v=ncUmWthHrU0
https://www.youtube.com/watch?v=Jcan8YfLfLs
https://www.reddit.com/r/linux/comments/b4zig/asklinux_how_does_grub_work/
https://0xax.github.io/grub/
https://www.youtube.com/watch?v=qIEGavnI-B4
https://www.youtube.com/watch?v=ZtVpz5VWjAs
https://www.gnu.org/software/grub/manual/grub/grub.html
https://www.youtube.com/watch?v=mHB0Z-HUauo




#########################################################################
################## 16 bit real mode #####################################
#########################################################################

So when running the code above, I would like to mention that we are in 16-bit real mode 

BIOS boots us into 16-bit real addressing mode. People also call it real mode. 
quoting this link: https://wiki.osdev.org/Real_Mode

        "Real Mode is a simplistic 16-bit mode that is present on all x86 processors. 
        Real Mode was the first x86 mode design and was used by many early operating systems before the birth of Protected Mode. 
        For compatibility purposes, all x86 processors begin execution in Real Mode."

So essentially to have backward compatibility, the solution Intel used is to emualte the oldest CPU in the family: 
the Intel 8086, which ad support for 16-bit instructions and no notion of memory protection. Later on we will 
be switching into 32 bit or 64 bit protected mode. 



########################################################################
############################# Print out Hello ##########################
########################################################################

So now to output characters on the screen, we will be using interrupts. 
Interrupts are a mechanism that allow the CPU temporarily to halt what it is doing and
run some other, higher-priority instructions before returning to the original task. An
interrupt could be raised either by a software instruction (e.g. int 0x10) or by some
hardware device that requires high-priority action (e.g. to read some incoming data from a network device).

So when the BIOS starts up, it sets up a table at the start of memory (at physical memory addres 0x0) 
you can actually see the address of each entry in the table
https://www.matrix-bios.nl/system/ivt.html

each entry occupies 4 bytes and each entry contains address pointers to interupt service routines (ISRs);
An ISRs is simply some code that handls a specific interrupt. So the BIOS calls the ISRs based on the index 
based into one of the CPU_s general purpose registers, ax. 


so lets get into coding. To type out 'Hello', we will do 'Write Character in TTY mode' BIOS Interrupt Call

http://3zanders.co.uk/2017/10/13/writing-a-bootloader/
https://github.com/cfenollosa/os-tutorial/tree/master/02-bootsector-print


                mov ah, 0x0e ; tty mode
                mov al, 'H'
                int 0x10
                mov al, 'e'
                int 0x10
                mov al, 'l'
                int 0x10
                int 0x10 ; 'l' is still on al, remember?
                mov al, 'o'
                int 0x10

                jmp $ ; jump to current address = infinite loop

                ; padding and magic number
                times 510 - ($-$$) db 0
                dw 0xaa55 



-   Quoting Wikipedia  
        a note about BIOS interrupt calls. BIOS only runs in the real address mode (Real Mode); of the x86 CPU, 
        so programs that call BIOS either must also run in treal mode or must switch from protected mode 
        to real mode before calling BIOS and then switching back again. For this reason, modern operating systems that 
        use the CPU in Protected mode generally do not use the BIOS to support system functions. 


        invoking an interrupt can be done using the INT x86 assembly language instruction. For example, to print a character 
        to the screen using BIOS interrupt 0x10, the following x86 assembly language instructions could be executed 

                mov ah, 0x0e    ; function number = 0Eh : Display Character
                mov al, '!'     ; AL = code of character to display
                int 0x10        ; call INT 10h, BIOS video service

        https://en.wikipedia.org/wiki/BIOS_interrupt_call

so essentially if you go into protected mode, you lose the BIOS interrupt calls

in the wikipedia page, you can also see it has the Interrupt table. 
here to put characters on the screen, its generally in the format of 


        ah                  al 

        _ _ _ _ _ _ _ _     _ _ _ _ _ _ _ _


we put ah with 0x0e, that tells the BIOS that we want video services 
we put al with the actual character data, that tells the BIOS the actual character we want to display. 




-   times 510 - ($-$$) db 0

$ means current address 
$$ means the first address of the current section. 
so $-$$ gives you the offset from start to address of the currently executed instruction. 

https://engineersasylum.com/t/times-510-db-0-means/132



-   db
db, dw, dd, dq, dt, do, dy and dz are all used in nasm to define varaibles 

db is declare bytes 
dw is declare word 
dq is declare 8 byte constant
dt is declare extended-precision float




with this code, our whole bootsector program now looks like: 

b4 0e b0 48 cd 10 b0 65 cd 10 b0 6c cd 10 b0 6c
cd 10 b0 6f cd 10 e9 fd ff 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
***************** a bunch of 0s ***************
00 00 00 00 00 00 00 00 00 00 00 00 00 00 55 aa


#####################################################################
################## BootSector Memory Location #######################
#####################################################################


another thing is that all code run by the CPU is somehwere in. The CPU fetches and executes instructions from memory. 
So for our boot sector code, we are initially in the hard drive. The BIOS loaded our 512-byte boot sector into memory and then, having finished 
its initialisations, told the CPU to jump to the start of our code, whereupon it began executing our first instructions,
then the next, then the next, etc. 

As it turns out, BIOS like always to load the boot sector to the address 0x7c00



so try this tutorial to really understand that the boot sector is at 0x7c00
https://github.com/cfenollosa/os-tutorial/tree/master/03-bootsector-memory



instead of manually every address with 0x7c00, what you can do is to use a ORG directive, 
to specify the origin address which NASM will assume the program begins at when 
it is loaded into memory. 
https://nasm.us/doc/nasmdoc7.html





#####################################################################
####################### BootSector Stack ############################
#####################################################################

https://github.com/cfenollosa/os-tutorial/tree/master/04-bootsector-stack

essentially we have ebp and esp registers 

ebp points to the base of your stack 
esp points to the top of your stack

     low memory 
     ___________      0x00000000
    |           |
    |           |
    |___________|
    |           |  <----- esp   
    |___________|   
    |           |   ^
    |___________|   |
    |           |   |
    |___________|   |
    |           |
    |___________|  <----- ebp 
    |           |
    |           |

     high memory 


so in the code here, we have 




     low memory 
     ___________      0x00000000
    |           |
    |           |
    |___________|
    |           |    
    |___________|   
    |     C     |  <----- esp 
    |___________|   
    |     B     |   
    |___________|   
    |     A     |
    |___________|  <----- ebp   0x8000
    |           |
    |           |

     high memory 



-   [] in nasm 
square brackets essentially work like a dereference operator (e.g., like * in C);

so 

                mov  REG, x 

moves the value of x into REG
whereas 
                
                mov REG, [x]

moves the value of the memory location where x points into in REG. 
Note that if x is a label, its value is the address of that label 





#####################################################################
####################### Reading Disk Sectors ########################
#####################################################################


As previously mentioned, our OS wont fit inside the bootsector 512 bytes, so we need to read 
data from a disk in order to run the kernel. 


https://github.com/cfenollosa/os-tutorial/tree/master/07-bootsector-disk

The main thing being used here is the BIOS interrupt calls on the disk serivce. 
recall that int 10h gave us the video services, 

int 13h gave us the Low Level Disk Services
https://en.wikipedia.org/wiki/BIOS_interrupt_call
https://en.wikipedia.org/wiki/INT_13H


so apparenlty, if you want to read data from the disk using the BIOS itnerrupt calls, 
you have to set a shit ton of register values. 
Details can be seen here 

http://stanislavs.org/helppc/int_13-2.html

so you have to set 
                
                AH to be 0x02
                AL to the number of sectors you want to read 
                CH the track/cylinder number 
                CL sector number 
                DH head number
                DL drive number 
                ES:BX pointer to buffer




in the tutorial, in boot_sect_main.asm, you can see that 



                [org 0x7c00]
                    mov bp, 0x8000 ; set the stack safely away from us
                    mov sp, bp

                    mov bx, 0x9000 ; es:bx = 0x0000:0x9000 = 0x09000
                    mov dh, 2 ; read 2 sectors
                    ; the bios sets 'dl' for our boot disk number
                    ; if you have trouble, use the '-fda' flag: 'qemu -fda file.bin'
                    call disk_load


we called 
                
                mov bx, 0x9000

to set the "pointer to buffer" when we do our disk read call. 



-   pusha, popa 
pusha and popa means push all register values and pop all register values. 







































So when the GRUB loads in a kernel, it expects the kernel to be of a certain format. 
The format that the kernel follows is the Multiboot specification. The multiboot is a standard 
to which GRUB expects a kernel to comply. It is a way for GRUB to 

1.  know exaclty what environment the kernel wants/needs when it boots 
2.  allow the kernel to query the environment it is in. 

So for example, if your kernel needs to be loaded in a specific VESA mode, you can inform the bootloader of this, 
and it can take care of it for you. 

So to make your kernel multiboot compatible, you need to add a header structure somwhere in your kernel (actually 
the header must be in the first 4KB);






##########################################################################################
########################################### actual code ##################################
##########################################################################################


so the first thing is that in our bootloader, we will have a multiboot header. 

Multiboot is about loading various kernels using a single boot loader. 
The Multiboot Header is a data structure in the kernel image that provides 
information to a Multiboot-compliant boot loader (GRUB for instance) about how and where to load the image, and which Multiboot features the image expects.

The multiboot header exists to allow a bootloader (for example, GRUB); to load the kernel to whom the header belongs in a way that that kernel expects.

For example, GRUB can provide the kernel with a memory map when it starts, but it would be a waste of time for GRUB to do that if the kernel it is loading wont use it. 
The flags field in the multiboot header is used to specify whether or not the kernel being loaded expects a map.
The multiboot header also includes the Multiboot magic number (which allows GRUB to find the location of the multiboot header).
In short, information in the header will either change how the kernel will be loaded into memory or request that the kernel would like some extra information.






so about the boot.s file 

1.  in the first section you see a bunch of .set calls 


                boot.s

                /* Declare constants for the multiboot header. */
                .set ALIGN,    1<<0             /* align loaded modules on page boundaries */
                .set MEMINFO,  1<<1             /* provide memory map */
                .set FLAGS,    ALIGN | MEMINFO  /* this is the Multiboot 'flag' field */
                .set MAGIC,    0x1BADB002       /* 'magic number' lets bootloader find the header */
                .set CHECKSUM, -(MAGIC + FLAGS) /* checksum of above, to prove we are multiboot */


apparenlty that is one of the assembly directives. Directives are commands that are part of the assembler syntax 
but are not related to the x86 processor instruction set. All assembler directives begin with a period (.) (ASCII 0x2E).

apparently, directives are available for the GNU assembler regardless of the target machien configuration. 
so assembler directives is just a GNU assembler thing. 

links: 
http://web.mit.edu/gnu/doc/html/as_7.html
https://docs.oracle.com/cd/E26502_01/html/E28388/eoiyg.html

You can see a list of directives in the links above:


        .set symbol, expression
        The .set directive assigns the value of expression to symbol. Expression can be any legal expression that evaluates to a numerical value.

so essentially we are literally declaring macros 

also notice in the 4th line, it says: 

                
                .set MAGIC,    0x1BADB002


that magic number is actually a magic number in the multiboot specification
see link below: 
https://wiki.osdev.org/Multiboot



2.  then the file declares a bunch of header 


                .section .multiboot
                .align 4
                .long MAGIC
                .long FLAGS
                .long CHECKSUM


here again, we see 3 directives. Lets see what they do:

        .section section, attributes
        The .section directive makes section the current section. 
        If section does not exist, a new section with the specified name and attributes is created. 
        If section is a non-reserved section, attributes must be included the first time section is specified by the .section directive.


        .align integer, pad
        The .align directive causes the next data generated to be aligned modulo integer bytes. 
        Integer must be a positive integer expression and must be a power of 2. If specified, pad is an integer bye value used for padding. 
        The default value of pad for the text section is 0x90 (nop); for other sections, the default value of pad is zero (0).


        .long expression1, expression2, ..., expressionN
        The .long directive generates a long integer (32-bit, two_s complement value) for each expression into the current section. 
        Each expression must be a 32–bit value and must evaluate to an integer value. The .long directive is not valid for the .bss section.



so as you can see we are defining 3 32-bit values. The first one is the magic value. In the link, and the comments, it says,
        
        "the bootloader will search for this signature in this first 8 kiB of the kernel file, aligned at a 32-bit boundary." 




3.  then the 3rd section, we have 

                
                .section .bss
                .align 16
                stack_bottom:
                .skip 16384 # 16 KiB
                stack_top:


so the comments section says: 

        /*
        The multiboot standard does not define the value of the stack pointer register
        (esp) and it is up to the kernel to provide a stack. This allocates room for a
        small stack by creating a symbol at the bottom of it, then allocating 16384
        bytes for it, and finally creating a symbol at the top. The stack grows
        downwards on x86. The stack is in its own section so it can be marked nobits,
        which means the kernel file is smaller because it does not contain an
        uninitialized stack. The stack on x86 must be 16-byte aligned according to the
        System V ABI standard and de-facto extensions. The compiler will assume the
        stack is properly aligned and failure to align the stack will result in
        undefined behavior.
        */


so as you can see, we are effective creating a stack like below:


                |               |
                |               |
                |               |
                |               | <---- stack_bottom 
                |               |
                |               |
                |               |
                |   16 KB       |
                |               |
                |               |
                |               |
                |               |
                |               |
                |               | <---- stack_top
                |               |
                |               |
                





-   the System V ABI standard

so in the comments it mentioned the "System V ABI standard". Apparently there is a osdev link on this topic.
https://wiki.osdev.org/System_V_ABI

The System V Application Binary Interface is a set of specifications that detail calling conventions, object file formats, executable file formats, dynamic linking semantics, 
and much more for systems.  It is today the standard ABI used by the major Unix operating systems such as Linux, the BSD systems, and many others. 
The Executable and Linkable Format (ELF) is part of the System V ABI.




4.      lets look at the next section 


                .section .text
                .global _start
                .type _start, @function



so here we see two new directives: 

        .global symbol, .globl symbol
        .global makes the symbol visible to ld. If you define symbol in your partial program, 
        its value is made available to other partial programs that are linked with it. 
        Otherwise, symbol takes its attributes from a symbol of the same name from another file linked into the same program.

        Both spellings (`.globl' and `.global') are accepted, for compatibility with other assemblers.

        On the HPPA, .global is not always enough to make it accessible to other partial programs. 
        You may need the HPPA-only .EXPORT directive as well. See section HPPA Assembler Directives.

ld, as mentioned is the GNU linker


we also have 

        .type int
        This directive, permitted only within .def/.endef pairs, records the integer int as the type attribute of a symbol table entry.

        '.type' is associated only with COFF format output; when as is configured for b.out output, it accepts this directive but ignores it.



the comments says 
        /*
        The linker script specifies _start as the entry point to the kernel and the
        bootloader will jump to this position once the kernel has been loaded. It
        doesn't make sense to return from this function as the bootloader is gone.
        */

so i guess, .global _start is making the "_start" symbol visible to the GNU linker. 




5.  now we go into the _start: section 
the things we are doing here is: 

-   so what we are doing here is that we set the esp register to point to the top of the stack. 

-   disable interrupts with cli (clear interrupt enable in eflags);



                _start: 
                    mov $stack_top, %esp
                    call kernel_main
                    cli
                1:  hlt
                    jmp 1b





6.  
                .size _start, . - _start

































A hard disk is essentially an array of blocks, each 512B in size. Could be over a billion of them. Numbered from 0 on up. 
Thats it. The partitioning schema runs over this, and filesystems are stored in those containers, and files are stored in there. Your BIOS is very dumb, 
it cant understand partitions, filesystems, files, and it especially cannot load an operating system. 
Remember that a program can only run in memory, and your OS is on a disk. So BIOS loads and runs the first block of the device you tell it to. 
Thats just 512B. Of that, the DOS partition table takes up like 90B, so you have 400B or so worth to put the start of your boot code. 
Thats not a lot of space. Fortunately, your first partition usually starts on the 64th block (which is sector #63). 
Thats because the traditional set up had 63 sectors per cylinder, leaving the entire first cylinder empty. So traditional grub would have stage 1 in sector 0, 
and stage "1.5" in the rest of that first cylinder. Those extra 61 sectors is around 30kB, enough room to cleverly store how to read from ext2 and ext3 partitions, 
which is where you have unlimited room for the meat of grub. Grub reads the /boot partition that you had configured it to use, 
and can get to actual files on a filesystem, and can learn how to use xfs, reiser, whatnot. 
Im pretty sure this is stage 2? But now grub is smart enough to read its configuration file, 
and load your linux kernel from your boot partition. Loading an OS is not trivial, it has to be placed into memory, 
and then you have to load parameters to the OS into certain regions of memory, and then tell the kernel information 
about where you loaded it, and then tell the CPU to jump to it, but you cant jump into it anywhere. You have to point into the correct entry point.

The difficult thing is that your cpu starts in real mode, which allows for unfettered handing off of control, 
but your cpu needs to be in protected mode in order to access the entirety of ram. So it toggles back and forth 
between these modes in order to load the kernel into ram properly. Ive heard this referred to as "unreal mode". 
Every OS used to have its own method for being loaded, so every OS used to have its own bootloader. 
The idea behind GRUB, or the grand unified boot loader, would be to be able to boot everything. They publish the multiboot specification, which many free OS follow. 
Its also smart enough to know how to load or chainload other operating systems. It cant load windows, but it can point to the microsoft bootloader that can.

This complexity is not just left to Linux. Traditionally, the DOS boot sector would look at the partition table, 
see which partition is flagged as bootable, and load that one. The start of those partitions would have a VBR, 
a volume boot record. That gives it 512B, and Im pretty sure they could then use additional sectors if they needed. 
This bootloader would then know how to load your OS, be it command.com or ntldr. 
ntldr is the real microsoft bootloader that knows how to genuinely get your windows kernel going.

So it really is a big complex chain of command.

Going forward though, this could all change. The planned replacement to BIOS, EFI, 
comes with its own replacement to the traditional MBR/DOS partitioning system. There arent only multiple filesystems in the world, 
there are multiple partition schemes. I think traditional macs and solaris did things their own way there. 
EFI_s partitioning is called GPT. EFI puts a smarter bootloader into the motherboard, so you can technically go without a bootloader in the MBR. 
EFI would let you select your partition and boot that. You dont need EFI to use GPT either, you can use it with BIOS and Grub2, right now.

One of the reasons that the largest HD_s we_ve seen has kind of stayed at 2TB for longer than usual is that it_s the largest that MBR partitioning can address. 
MBR partitions start and end numbers are fixed at 32bit, which means you can_t have any partition extending past the first 2TB of a drive.

GPT leaves the first sector blank, for backwards compatibility and for safety. Then it uses the next 62 sectors to store the partition table. This is a lot larger than the old 90B partitioning table you used to have. It is 64 bit, and allows for 128 raw partitions,
 instead of just 4. It does away with MBR_s "extended" partitioning which used a complex linked list of EBRs. If you want to use GPT but have BIOS instead of EFI, you can. Grub2 can install to the first sector (conveniently left blank), and you make one of your 128 partitions a tiny 31kB partition just for GRUB, using FAT. Your BIOS will boot grub2, grub2 will load more information from the grub_boot partition, and then it can find the /boot where it finds its config and presents a menu, and it can then chainload or load a kernel or boot usb or whatever you need.














so there is this Multiboot specification, which is a specification of how to .....



        "Bootloader is a piece of code/program that runs before an operating system starts to run. 
        It loads an operating system when a computer is turned on. It tells the hardware where to look and how to get running when you start things up.

        So in this tutorial, you will be using GRUB. 

        GRUB is the GNU projects bootloader. 
        https://www.cs.tau.ac.il/telux/lin-club_files/linux-boot/slide0002.htm"


so in the description above, you can see that the bootloader will have to run some code to load our OS. 
in our minimal OS bare bones tutorial, that "some code" is gonna be our boot.s. 
That boot.s is gonna tell the GRUB, the bootloader how to load and start our kernel.c. So that is the basic idea. 

Luckily there is the Multiboot standard, "which describes an easy interface between the bootloader and the operating system kernel"








-   multi-booting 

Multiboot is about loading various kernels using a single boot loader.
installing multiple OS on a computer, and being able to choose which one to boot. 


https://stackoverflow.com/questions/17698121/what-is-meant-by-multiboot-header



-   multiboot standard

So there is a multiboot specification. It is an open standard that provides kernels with a uniform 
way to be booted by Multiboot-compliant bootloaders. 

The Multiboot specification is an open standard describing how a boot loader can load an x86 operating system kernel.
The specification allows any compliant boot-loader implementation to boot any compliant operating-system kernel. 






https://stackoverflow.com/questions/23854755/who-defines-the-bootloader-specification



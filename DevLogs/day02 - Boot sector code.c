
So the tutorial I am following is: 
https://www.cs.bham.ac.uk/~exr/lectures/opsys/10_11/lectures/os-dev.pdf
https://github.com/cfenollosa/os-tutorial


##########################################################################################
################################## bootloader ############################################
##########################################################################################

a bit of background of how a bootloader works.

so imagine your motherboard looks like this:

on the motherboard, there is a chip that contains a bit of code. That piece of code is called BIOS.
The BIOS is stored in ROM (read only memory);, and it is accessed for the first time by the CPU. 
BIOS is Basic Input Output System. Essentially, the BIOS code is baked into the motherboard of your PC. 

(A side note: the CPU can only receive instructions directly from either the BIOS or the RAM);

https://stackoverflow.com/questions/20861032/who-loads-the-bios-and-the-memory-map-during-boot-up

so a lot of times nowadays, since RAM is faster than ROM, the BIOS is copied to RAM and run it from there.
Note that the copy isnt done by some magic circuitry, it is just done by the BIOS itself when it 
starts executing out of ROM initially, it just copies itself to RAM and then continues executing from there. 
https://superuser.com/questions/336021/is-bios-read-from-the-bios-chip-or-copied-into-ram-on-startup
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
https://github.com/cfenollosa/os-tutorial/tree/master/01-bootsector-barebones



Now to run this our bootloader code, we can just run the line 

                nasm -f bin boot_sect_simple.asm -o boot_sect_simple.bin

for all the compile flags, try 

                nasm -help 

the -f bin option is used to instruct nasm to produce raw machine code, rather than a code 
package that has additional meta information for linking in other routines that we would expect to use 
when programming in a more typical Operating system enviornment 

"-o outfile", write output to an outfile



############################################################
############################## QEMU ########################
############################################################
and then we run it in qemu 
                
                qemu-system-x86_64 boot_sect_simple.bin


so to understand what is going on, so if you look at the qemu command 
                
                qemu-system-x86_64 [disk_image]

disk_image is essentially a binary map of the hard disk. so when we compile our boot sector code as a binary code,
we give it to qemu, and qemu will interpret it as a hard disk.


I found this link for QEMU options 
https://wiki.gentoo.org/wiki/QEMU/Options

-hda IMAGE.img - Set a virtual hard drive and use the specified image file for it.















https://neosmart.net/wiki/mbr-boot-process/
if you think about it, 440 bytes is very little, so usually what happens is that you have to look up 
another file from the disk and load it to perform the actual boot process. So often times 
the job of the bootstrap code segment in the MBR is pretty simple: look up the active partition from the partition 
table, and load that code into the memory for the execution by the CPU as the next link in the boot chain. 

so it goes from the BIOS to the bootstrap code in the MBR, then from the MBR to the bootstrap code in the partition bootsector, 
and from there to the executable boot files on the active partition, the actual logic involved in determining which
 operating system to load, where to load it from, which parameters/options to pass on to it, 
 and completing any interactions with the user that might be available, the actual process of starting the operating system begins.

So the actual bootloader files on the disk forms the final parts of the boot loading process. When people talk about 
bootloaders and boot files, they are often referring to this final, critical step of the boot process. 


So almost all bootloaders separate the actual, executable bootloader from the configuration file or database 
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

so when MBR read your USB stick, it will see your GRUB program and it load it 
then the GRUB in turn loads the multple Kernel that you wish to load. 
visually it looks like 

                
                 Partition Table         Disk Partitions
                    |  |  |            /        |        \
                    |  |  |           /         |         \
                    v  v  v          v          v          v
         _____________________________________________________________
        |  MBR     |  |  |  |             |             |             |                                                        
        |__________|__|__|__|_____________|_____________|_____________|

                                ^
                                |
                                |
             GRUB is somewhere here in your USB stick along with your OS. 





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

All x86 compatible boot into 16 bit mode. 

-   RAM avilable

So one thing that is quite trick is that when the computer is in 16 bit real mode, you would think that the computer would have 
64 kb of memory since 

16 bit is 10000 in hex, 65536 in decimal, 10000000000000000 in binary 

but one thing you might not know is that in 16 bit mode, addressing is done using SS and SP (Stack segment and stack pointer);
so its the SS:SP combo with SS pointing to the stack and SP pointing to the stack pointer. 
so its 20 bit wide addressing

hence the amount of memory is actually 2^20, which gives you 1 MB 

Essentially, in 16 bit mode, we are limited to 1 MB (+64k) of memory. 

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


another thing is that all code run by the CPU is somehwere in memory. The CPU fetches and executes instructions from memory. 
So for our boot sector code, they are initially in the hard drive. The BIOS loaded our 512-byte boot sector into memory and then, having finished 
its initialisations, told the CPU to jump to the start of our code, whereupon it began executing our first instructions,
then the next, then the next, etc. 

As it turns out, BIOS like always to load the boot sector to the address 0x7c00


so try this tutorial to really understand that the boot sector is at 0x7c00
https://github.com/cfenollosa/os-tutorial/tree/master/03-bootsector-memory



instead of manually every address with 0x7c00, what you can do is to use a ORG directive, 
to specify the origin address which NASM will assume the program begins at when 
it is loaded into memory. 
https://nasm.us/doc/nasmdoc7.html


To give you an idea where 0x7c00 is in memory 

0x7c00 is 11111 0000000000 in binary 
which is 31 KB

                 ___________________    <--------- 0x00000000 
                |                   |
                |                   |
                |                   |
                |___________________|   <--------- 0x00007C00 (31 kb)         
                |    bootsector     |
                |                   |    bootsector is 512 bytes
                |- - - - - - - - - -|
                |                   |
                |___________________|  <--------- 0x00008000 (32 kb)
                |                   |
                |                   |                                
                |                   |
                |                   |
                |                   |                                
                |                   |
                |                   |
                |                   |                                
                |                   |
                |___________________|   <---------- 4 GB (not drawn to scale)
                                

as you can see, its 1 kb short of 32 kb. The reason why the bootsector code is at 0x7c00 is becuz 0x7c00 is 1k 
(512 bytes for the bootsector plus an additional 512 bytes for possible bootsector use) 
from the bottom of the original 32k installed memory. 

Essentially we left another 512 bytes for the bootsector just in case it needs to use it.

https://stackoverflow.com/questions/2058690/what-is-significance-of-memory-at-00007c00-to-booting-sequence





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






so here, we are setting up our stack to be at 0x8000



                 ___________________    <--------- 0x00000000 
                |                   |
                |                   |
                |                   |
                |___________________|   <--------- 0x00007C00 (31 kb)         
                |    bootsector     |
                |                   |    bootsector is 512 bytes
                |- - - - - - - - - -|
                |                   |
                |___________________|   <--------- 0x00008000 (32 kb)
                |                   |
                |       stack       |                                
                |                   |
                |___________________|
                |                   |                                
                |                   |
                |                   |
                |                   |
                |                   |
                |                   |                                     
                |                   |
                |                   |                                     
                |                   |
                |___________________|   <---------- 4 GB (not drawn to scale)
                                




#####################################################################
####################### Reading Disk Sectors ########################
#####################################################################


As previously mentioned, our OS wont fit inside the bootsector 512 bytes, so we need to read 
data from a disk, put it into memory and begin executing that code. 

So you can think of this as a Second Stage bootloader. If we load all sectors for the 
Second Stage loader in a good manner, the Second Stage Loader has no limitation in size. 

https://www.cs.bham.ac.uk/~exr/lectures/opsys/10_11/lectures/os-dev.pdf
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




There is also this link
http://www.brokenthorn.com/Resources/OSDev5.html





in the tutorial, in boot_sect_main.asm, you can see that 



                [org 0x7c00]
                    mov bp, 0x8000 ; set the stack safely away from us
                    mov sp, bp

                    mov bx, 0x1000 ; es:bx = 0x0000:0x1000 = 0x1000
                    mov dh, 2 ; read 2 sectors
                    ; the bios sets 'dl' for our boot disk number
                    ; if you have trouble, use the '-fda' flag: 'qemu -fda file.bin'
                    call disk_load


we called 
                
                mov bx, 0x1000

to set the "pointer to buffer" when we do our disk read call. 


then inside disk_load, we have:


                disk_load:
                    pusha
                    push dx 

                    mov ah, 0x02        ;   bios read sector function
                    mov al, dh          ;   read dh sector 
                    mov ch, 0x00        ;   select cylinder 0 
                    mov dh, 0x00        ;   select head 0 
                    mov cl, 0x02        ;   start reading from second sector 
                    int 0x13            ;   BIOS interrupt 

                    jc disk_error       ;   jump if error 

                    pop dx
                    cmp dh, al 
                    jne disk_error 
                    popa
                    ret 

                disk_error:
                    mov bx, DISK_ERROR_MSG 
                    call print
                    jmp $

                DISK_ERROR_MSG  db "Disk read error!", 0



notice the "pusha and popa" pairing




-   pusha, popa 
pusha and popa means push all register values and pop all register values. 





So here, we are putting our OS to be at 0x1000



                 ___________________    <--------- 0x00000000 
                |                   |
                |                   |
                |                   |
                |___________________|   <--------- 0x00007C00 (31 kb)         
                |    bootsector     |
                |                   |    bootsector is 512 bytes
                |- - - - - - - - - -|
                |                   |
                |___________________|   <--------- 0x00008000 (32 kb)
                |                   |
                |       stack       |                                
                |                   |
                |___________________|   <--------- 0x00009000 (36 kb) 
                |                   |
                |       kernel      |                                
                |                   |
                |___________________|                
                |                   |                                
                |                   |
                |                   |
                |                   |
                |                   |
                |                   |                                     
                |                   |
                |                   |                                     
                |                   |
                |___________________|   <---------- 4 GB (not drawn to scale)
                                







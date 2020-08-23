################################################################
################ Loading Kernel at 1 MB ########################
################################################################









################################################################
################ Disk Load Sections 2 ##########################
################################################################
So remember in day 07, where I changed the number of sectors to 10 and added a bunch of padding at the end?

                [bits 16]
                load_kernel:
                    mov bx, MSG_LOAD_KERNEL
                    call print 

                    mov bx, KERNEL_OFFSET
    ----------->    mov dh, 1
                    mov dl, [BOOT_DRIVE]    ;   drive number (0=A:, 1=2nd floppy, 80h=drive 0, 81h=drive 1)
                    call disk_load
                    
                    ret 



                padding.asm 

                times 256 dw 0 ; sector 2 = 512 bytes
                times 256 dw 0 ; sector 3 = 512 bytes
                times 256 dw 0 ; sector 4 = 512 bytes
                times 256 dw 0 ; sector 5 = 512 bytes
                times 256 dw 0 ; sector 6 = 512 bytes
                times 256 dw 0 ; sector 7 = 512 bytes
                times 256 dw 0 ; sector 8 = 512 bytes
                times 256 dw 0 ; sector 9 = 512 bytes
                times 256 dw 0 ; sector 10 = 512 bytes


Well, turns out 10 sectors is not enough.

So by looking at the brokenthorn OS dev series 
http://www.brokenthorn.com/Resources/OSDev5.html

it seems like to really determine how many sectors to load, I will have to look at a file system.
Which means I need to harddrive that contains a file system.

currenlty I am using qemu and loading raw binary into the harddrive using the -hda option.


so my options are
1.  boot from floppy disk, follow Brokenthorn tutorial

however, this links https://wiki.osdev.org/Babystep1 
says we should just forget about floppy disks all together. 


2.  Hard disk and set its partitions
We will use this method

Recall in day 2, we drew this diagram 


Master Boot Record is the bootsector of a hard disk. 




                 Partition Table         Disk Partitions
                    |  |  |            /        |        \
                    |  |  |           /         |         \
                    v  v  v          v          v          v
         _____________________________________________________________
        |  MBR     |  |  |  |             |             |             |                                                        
        |__________|__|__|__|_____________|_____________|_____________|


I found the link below that describes the MBR and partition table in more detail 
https://wiki.osdev.org/MBR_(x86)

so for a hard dirve, we actually only get 440 bytes for the code 


                Offset              Size (bytes)    Description
                0x000               440             MBR Bootstrap (flat binary executable code)
                0x1B8 (440)         4               Optional "Unique Disk ID / Signature"2
                0x1BC (444)         2               Optional, reserved 0x00003
                0x1BE (446)         16              First partition table entry
                0x1CE (462)         16              Second partition table entry
                0x1DE (478)         16              Third partition table entry
                0x1EE (494)         16              Fourth partition table entry
                0x1FE (510)         2               (0x55, 0xAA) "Valid bootsector" signature bytes



                Offset1 Size (bytes)    Description
                0x00    1               Drive attributes (bit 7 set = active or bootable)
                0x01    3               CHS Address of partition start
                0x04    1               Partition type
                0x05    3               CHS address of last partition sector
                0x08    4               LBA of partition start
                0x0C    4               Number of sectors in partition




To write an MBR to the first sector of a disk, you must use special disk I/O tools, 
because the MBR (by definition) is not inside any disk partition. 
The MBR only exists on the "raw device". There are quite a few "disk editing" tools available; some are listed in Disk Image Utilities.



Here I use dd command line that works on UNix, since Cygwin is a unix emulator



so in the link above 
https://wiki.osdev.org/MBR_(x86)

it gave some sample code under the "x86 Examples" section.

https://en.wikipedia.org/wiki/Master_boot_record#cite_note-Sedory_2004_Timestamp-3








###############################################################
####################### Partition Table #######################
###############################################################

The Master Boot Record is the traditional way of storing partition information about a hard disk, along with some boot code. 
That is, the Partition Table is contained inside the MBR, 
which is stored in the first sector (cylinder 0, head 0, sector 1 -- or, alternately, LBA 0) of the hard drive. 
(See the MBR article for the overall structure and contents of the MBR.)

Almost all PCs still use an MBR for booting hard disks, and for storing partition information on hard disks. 
Traditional MBRs are nearly obsolete at this time, because the 32 bit design of the LBA fields in the 
Partition Table begins to "overflow" when dealing with disks larger than 2Tb. The successor to the MBR system is GPT (see below).



So there is MBR disk and there is GPT disk.
we will create a MBR disk 

        /sbin/fdisk diskimage.dd



https://wiki.archlinux.org/index.php/Partitioning#Master_Boot_Record


so if you type "m" in fdisk, you will see 

                Create a new label
                    g   create a new empty GPT partition table
                    G   create a new empty SGI (IRIX) partition table
                    o   create a new empty DOS partition table
                    s   create a new empty Sun partition table

for our purpose, we will use 'o'. On the wiki 
    https://wiki.archlinux.org/index.php/Partitioning#Master_Boot_Record

it says "the MBR partition table (also known as DOS or MS-DOS partition table)"


so now we want to split our file into two files 

1.  the bootsector code, which is the top 512 bytes 
2.  the kernel. We need to add this into our partition. 



https://answers.microsoft.com/en-us/windows/forum/windows_xp-performance/what-is-the-difference-between-the-c-drive-and-d/1817f002-7981-41ea-afb5-7e28ff3235c5

this is like in windows where you can partition the hard drive into any number of partitions 
C,D,E for use as data storage. 

so you can think of this way Harddrive:

    A:
    C:
    D:
    E:

four partitions. 

Each of the disk will need a file system to manage any files you put under the disk.




These two things are all you need: 1. A device to store/read the data 2. Rules on how to handle the data

Example: Lets say you would copy a binary to the beginning of your hdd and tell your computer to boot from this hdd. 
The computer will read the first command and execute it, and then read the next command and so on. 
This is what a bootloader does. At this early stage there are no filesystems, partitions, etc. involved.

At the beginning of software development you didnt 'open a File', you 'read bytes 100 to 180' and work with this 
data (maybe the 80 bytes are a string or audio data). Working with numbers got annoying (Where does my string start? 
Was it 40? How long was it again? Which string is this?), so Filesystems where invented:






what I realized is that, if you dont use a file system, you dont really know how many sectors to load.
unless you can create a disk that perfectly the number of sectors to hold your kernel.
but usually you first create an hard drive image, then partition it, and then create the number sections,
then finally put your kernel in. So its in reverse.









https://stackoverflow.com/questions/24429949/device-vs-partition-vs-file-system-vs-volume-how-do-these-concepts-relate-to-ea





https://wiki.osdev.org/Rolling_Your_Own_Bootloader

You will have to decide where in memory you are going to load your kernel. Your kernel generally depends on it.

In Real Mode, the easiest is to stay below the 1MB barrier, 
which means you practically have 512KB of memory to load things. 
You may wish the kernel to be loaded at a well-known position, say 0x10000 physical (es=0x1000, bx=0 when calling INT13h).

If your kernel is bigger (or is expected to grow bigger) than this, 
you wll probably prefer to have the kernel above the 1MB barrier, 
which means you need to activate A20 gate and switch to Unreal Mode to load the kernel 
(with A20 alone, you cannot have more than 64K above 1MB).

Note that BIOS will still be unable to write to memory above 1MB, 
so you need to read stuff in a buffer below 1MB and then perform a rep 
movsd to place the data where they ultimately should go.


why do I have 512 K of memory to load things?

I am assuming, they are refering to this memory map, where anything above 640 kb is reserved for devices.


                 _______________________________    <--------- 0x100000 (1MB)
                |                               |
                |                               |
                |         System BIOS           | 
                |                               |
                |                               |
                |_______________________________|   <--------- 0xF0000 (960 kb) 
                |                               |
                |       BIOS Shadow Area        |
                |_______________________________|   <--------- 0xC8000 (800 kb) 
                |                               |
                |       Video ROM BIOS and      |
                |_______________________________|   <--------- 0xC0000 (768 kb)
                |                               |
                |   Monochrome Video Memory and |   
                |       Color Video Memory      |
                |_______________________________|   <--------- 0xB0000 (704 kb)
                |                               |
                |         Legacy Video          |
                |       Card Memory Access      |
                |_______________________________|   <--------- 0xA0000 (640 kb)
                |                               |
                |                               |   
                |                               |      our stack
                |_______________________________|   <--------- 0x90000 (576 kb)          
                |                               |
                |                               |
                |                               |
                |                               |               
                |                               |
                |_______________________________|   
                |                               |   
                |           bootsector          |   
                |_______________________________|   <-------- 0x7C00 (31 kb)              
                |                               |
                |                               |
                |                               |
                |                               |
                |                               |   
                |                               |   
                |_______________________________|   <--------- 0x0 


so I put it at 

where to place our kernel 

so if you look at this map 
https://wiki.osdev.org/Memory_Map_(x86)

we usable memory from 0x500 ~ 0x7BFF (1.25 kb ~ 31 KB)

and from 0x7E00 ~ 0x7FFFF (31.5 kb ~ 512 kb)

so i am just gonna put my kernel at 0x1000 (4096) 
that means I have around 27 kb for my kernel 


and putting my stack 0x90000 (576 kb)


                 _______________________________    <--------- 0x100000 (1MB)
                |                               |
                |                               |
                |         System BIOS           | 
                |                               |
                |                               |
                |_______________________________|   <--------- 0xF0000 (960 kb) 
                |                               |
                |       BIOS Shadow Area        |
                |_______________________________|   <--------- 0xC8000 (800 kb) 
                |                               |
                |       Video ROM BIOS and      |
                |_______________________________|   <--------- 0xC0000 (768 kb)
                |                               |
                |   Monochrome Video Memory and |   
                |       Color Video Memory      |
                |_______________________________|   <--------- 0xB0000 (704 kb)
                |                               |
                |         Legacy Video          |
                |       Card Memory Access      |
                |_______________________________|   <--------- 0xA0000 (640 kb)
                |                               |
                |                               |   
                |                               |      our stack
                |_______________________________|   <--------- 0x90000 (576 kb)          
                |                               |
                |                               |
                |                               |
                |                               |               
                |                               |
                |_______________________________|   
                |                               |   
                |           bootsector          |   
                |_______________________________|   <-------- 0x7C00 (31 kb)              
                |                               |
                |                               |
                |                               |
                |                               |
                |                               |   
                |                               |   
                |_______________________________|   <--------- 0x0 



logical block addressing.
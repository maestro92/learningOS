
###############################################################################
############################# Video Memory ####################################
###############################################################################

so now we will write a new print function that works in protected mode. which doesnt work with BIOS interrupts,
but by directly manipulating the VGA video memory instead of calling the int 0x10.
The VGA memory starts at address 0xb8000 and it has a text mode which is useful to avoid manipulating direct pixels. 


So to give you an idea where 0xb8000 is at, 

You can refer to this graph
https://www.yourdictionary.com/pc-memory-map
or this one
https://www.slideserve.com/Olivia/pc-bootup

lets manually draw a memory map. lets assume we are 32 bit, which gives us 4GB of memory  

32 bit computers support 4GB cuz you can represent numbers up to [0, 2^32 - 1] 

2^32 is 4294967296 

hexidecimal means base 16. So each hex decimal represents 4 bits 

F is 1111

I was a bit rusty, so I had to brush up on some binary and hex math just to give some reference in bytes 

                    in Hex 
1 KB        2^10    400
1 MB        2^20    100000
1 GB        2^30    40000000



so 0xB8000 is 1011100000 0000000000 in binary
1011100000 is 736 

so this is 736 kb



so 0xBC000 is 1011110000 0000000000 in binary 

1011110000 is 752 
so this is 752 kb


as you can see, the VGA Text memory resides between 736 kb and 752 kb


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
                |                   |   <--------- 0x100000 (1MB)


https://manybutfinite.com/post/motherboard-chipsets-memory-map/

So this brings up to to the topic of memory map are also used for communication with assorted devices on the motherboard. 
this communication is called memory-mapped I/O (https://en.wikipedia.org/wiki/Memory-mapped_I/O)

These devices include video cards, most PCI cards (say a scanner, or SCSI card), and also flash memory that stores he BIOS 


So when the mother board receives physical memory requests, it decides where to rout it. 
The routing is decided via the memory address map. For each region of physical memory addresses, the memory map knows 
the device that owns the region. 

The link above has an image and shows the memory-mapped I/O.

Notice in the graph, the amount of actual RAM available is around 3GB




-   video memory on bootup
https://stackoverflow.com/questions/20861032/who-loads-the-bios-and-the-memory-map-during-boot-up

So on Day2, we talked about how BIOS calls into the bootstrap code. Today we will talk about memory map on boot up.


Recall that on day 2, we mentioned that BIOS copies itself into RAM, as you can image, BIOS also initializes the memory 
controller before it copies itself from ROM to RAM. 

The memory map layout varies from system to system. At power-on, the BIOS will query the attached PCI/PCIe devices,
determine what resources are needed and place them in the memory map at the optimcal location. 
If everything is working properly, memory-mapped devices should not overlap with RAM. 

Essentially people intel decides that for a certain model of computers, they bake the memory map information into the chipset 
and the BIOS knows about the memory map. 




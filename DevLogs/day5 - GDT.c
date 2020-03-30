So we already know that the GDT describes certain info about the parts of the memory 
its also a way to protect certain parts of the memory 

for example: 

lets same this is our 4GB memory, 
and in our GDT, we defined that code segment and data segment is below:

     ___________________
    |                   |
    |                   |
    |                   |
    |___________________|
    |                   |
    |                   |
    |    code segment   |
    |                   |
    |___________________|
    |                   |
    |    data segment   |
    |                   |
    |___________________|
    |                   |
    |                   |
    |___________________|

so in our GDT, we define where in memory that the allowed region will start, as well as the limit of this region,
and the access privileges associated with it.

if you are accessing instructions that is not in the code segment, you will trigger an exception.
same, if the CPU is accessing data from memory thats not in the data segment, this will protect it. 

So thats the idea with the GDT providing protection

http://www.osdever.net/bkerndev/Docs/gdt.htm


#############################################################################
################ Memory Addressing in Real vs Protected mode ################
#############################################################################

So in both real and protected modes, the system uses 16-bit segment registers to derive the actual memory address. 

(note, segment registers refer to any of the CS, DS, SS, ES, FS and GS. These are all segment registers).


in 16 bit real mode, we use the segment register + offset to address memory. so we get to have 20 bit addressing. 
https://en.wikipedia.org/wiki/X86_memory_segmentation


In protected mode, we use the segment register as in index into the GDT. The GDT is just a table of segment descriptor (SD);
each segment descritpor is an 8-byte structure that defines the following properties 

https://wiki.osdev.org/Global_Descriptor_Table

Details structure of the segment descriptor can be seen in the above link. 


###########################################################################
############## Segment Register configuration #############################
###########################################################################

so the simplest workable configuration of segment registers is described by Intel as the basic flat model,
whereby two overlapping segments are defined that cover the full 4 GB of addressable memory, one for code and the other 
for data. So for early on with our development, we just gonna do the most brute force and simple configuration. 

essentially both code segment and data segment overlaps. As you might have noticed, we are not actually protecting one segment from the other. 
But again, this is simple, and we like that.
     ___________________
    |                   |
    |                   |
    |                   |
    |                   |
    |                   |
    |                   |
    |    code segment   |
    |                   |
    |                   |
    |                   |
    |    data segment   |
    |                   |
    |                   |
    |                   |
    |                   |
    |___________________|


As a matter of fact, the x86-64 architecture requires a flat memory model (one segment with a base of 0 and a limit of 0xFFFFFFFF)
for some of its instructions to operate properly. 


Another thing to do is that the first entry in the GDT purposly be an invalid null descriptor (thats how null exceptions happen);
If an addressing attempt is made with the null descriptor, then the CPU wil raise an null exception.
Which essentially is an interrupt. (this interrupt is not to be confused with exceptions in higher level languages such as Java).


so x86 in 32 or 64 bit, you have 6 segmentation registers (CS, DS, SS, ES, FS and GS). 
Each holds an offset into the GDT. The code sgement must reference a descriptor which is set as a 'code segment'


another thing is that GRUB sets a GDT up for you.



###########################################################################
######################## GDT entry ########################################
###########################################################################


Each GDT entry also defines whether or not the current segment that the processor is running in is for System use 
(Ring 0) or for Application use (Ring 3). There are other ring types, but they are not important. 
Major operating systems today only use Ring 0 and Ring 3.
































###########################################################################
################################### Switching #############################
###########################################################################

We first tell the CPU about the GDT we just prepared 
        

                lgdt [gdt_descirptor]


Then we make the actual swtich, by setting the first bit of a special CPU control register, cr0.
                
                mov eax, cr0 
                or eax, 0x1
                mov cr0, eax 

after the cr0 has been updated, the CPU is in 32-bit protected mode. 














So you offically go into protected mode after the system sets up one descriptor table and enables the 
Protection Enable (PE) bit in the control register 0 (CR0).

https://www.cs.bham.ac.uk/~exr/lectures/opsys/10_11/lectures/os-dev.pdf





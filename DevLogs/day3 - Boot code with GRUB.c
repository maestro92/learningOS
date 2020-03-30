So as mentioned we will take a look at how GRUB works. 
using the graph we had from day 2 


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




So you can think of the relationship between GRUB and kernel as BIOS and MBRs.

Recall that 

        " The BIOS finds a target device to boot from that contains a master boot record(MBR); 
        usually from Disk Drive, SD Card Reader, CD/DVD Rom or Hard Drive;"

and for the BIOS to know that you are a bootable device, you have to put (0x55, 0xAA) as a magic number
at the last 2 bytes of the 512 bytes

Same thing with GRUB and your kernel. For GRUB to know that your program is a kernel that it can load, 
our kernel has to conform to the multiboot specification. To achieve this, you have to put the 'magic number', so 
that GRUB can find you.

It is a way for GRUB to 

1.  know exaclty what environment the kernel wants/needs when it boots 
2.  allow the kernel to query the environment it is in. 

So for example, if your kernel needs to be loaded in a specific VESA mode, you can inform the bootloader of this, 
and it can take care of it for you. 

So to make your kernel multiboot compatible, you need to add a header structure somwhere in your kernel (actually 
the header must be in the first 4KB);






##########################################################################################
########################################### actual code ##################################
##########################################################################################





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



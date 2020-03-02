So in day 1, we built our GCC Cross-Compiler. 



I went on to do the Bare_Bones tutorial: 
https://wiki.osdev.org/Bare_Bones

a couple of tools that will be used: 

-   The GNU Linker from Binutils to link your object files into the final kernel.
-   The GNU Assembler from Binutils (or optionally NASM) to assemble instructions into object files containing machine code.
-   The GNU Compiler Collection to compile your high level code into assembly.
-   The GRUB bootloader to bootload your kernel using the Multiboot boot protocol that loads us into 32-bit protected mode with paging disabled.
-   The ELF as the executable format that gives us control of where and how the kernel is loaded.


three input files 
boot.s - kernel entry point that sets up the processor environment
kernel.c - your actual kernel routines
linker.ld - for linking the above files



##########################################################################################
################################## bootloader and boot.s #################################
##########################################################################################

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

installing multiple OS on a computer, and being able to choose which one to boot. 


-   multiboot standard

So there is a multiboot specification. It is an open standard that provides kernels with a uniform 
way to be booted by Multiboot-compliant bootloaders. 

The Multiboot specification is an open standard describing how a boot loader can load an x86 operating system kernel.
The specification allows any compliant boot-loader implementation to boot any compliant operating-system kernel. 




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






























so following this link:

https://wiki.osdev.org/Raspberry_Pi_Bare_Bones


                "The first thing you should do is set up a GCC Cross-Compiler for arm-none-eabi."



##########################################################################
############################ arm-none-eabi ###############################
##########################################################################

so what is arm-none-eabi
https://web.eecs.umich.edu/~prabal/teaching/resources/eecs373/toolchain-notes.pdf

GCC is a popular, open source toolchain that can generate code for a wide range of architectures including Intel’s x86, 
ARM v4/5/6/7, TI’s MSP, Atmel’s AVR, and many others


 If you search for an ARM compiler, you
might stumble across the following toolchains: arm-none-linux-gnueabi, arm-none-eabi, arm-eabi, and
arm-softfloat-linux-gnu, among others. This might leave you wondering about the method to the naming
madness. Unix cross compilers are loosely named using a convention of the form arch[-vendor][-os]-abi.
The arch refers to the target architecture, which in our case is ARM. The vendor nominally refers to the
toolchain supplier. The os refers to the target operating system, if any, and is used to decide which libraries
(e.g. newlib, glibc, crt0, etc.)




• arm-none-eabi is the toolchain we use in this class. This toolchain targets the ARM architecture,
has no vendor, does not target an operating system (i.e. targets a “bare metal” system), and complies
with the ARM EABI.



ARM EABI

EABI is the new "Embedded" ABI by ARM ltd. EABI is actually a family of EBAIs and one of the "sbuABIs" is GNU EABI, for Linux. 

https://wiki.debian.org/ArmEabiPort






-   ABI 
https://en.wikipedia.org/wiki/Application_binary_interface

an ABI defines how data structures or computational routines are accessed in machine code, 


System V ABI is the standard ABI used by the major Unix Operating system such as Linux, the BSD systems, and many others. 
The Executable and Linkable Format (ELF) is part of the System V ABI. 











-   Linux 
Linux is perfect in this regard because nowadays everything from small IoT devices to large servers tend to run Linux.

https://github.com/s-matyukevich/raspberry-pi-os/blob/master/docs/Introduction.md


Each OS has 2 fundamental goals 
1.  Run user process in isolation
2.  Provide ach user process with a unified view of the machine hardware.




from this link 
https://hackaday.com/2018/01/19/roll-your-own-raspberry-pi-os/
                


                Writing an operating system is no small task, but like everything else it is easier than it used to be. 
                [JSandler] has a tutorial on how to create a simple operating system for the Raspberry Pi. 
                One thing that makes it easier is the development environment used. 
                QEMU emulates a Raspberry Pi so you can do the development on a desktop PC 
                and test in the virtual environment. When you are ready, you can set up a bootable SD card and try your work on a real device.


so lets download QEMU.




http://wiki.laptop.org/go/Using_QEMU_on_Windows


                The accelerator for QEMU named KQEMU, while not required, is highly recommended as a performance booster.


so lets also download KQEMU











#####################################################################################
####################################### QEMU ########################################
#####################################################################################

according to wikipedia 

                QEMU is Quick EMUlator is a free and open-source emulator that performs hardware virtualization. 


                QEMU is a hosted virtual machine monitor: it emulates the machines processor through dynamic binary translation 
                and provides a set of different hardware and device models for the machine, enabling it to run a variety of guest operating systems. 
                It also can be used with KVM to run virtual machines at near-native speed (by taking advantage of hardware extensions such as Intel VT-x). 
                QEMU can also do emulation for user-level processes, allowing applications compiled for one architecture to run on another.






#################################################################################
###################### Raspberry pi zero Specs ##################################
#################################################################################


Raspberry pi model comparisons

so the raspberrypi pi zero is in this link
https://socialcompare.com/en/comparison/raspberry-pi-alternatives

the raspberrypi zero w is in this link
http://socialcompare.com/en/comparison/raspberrypi-models-comparison


from looking at the model, two looks very similar to the one I have:

but it seems like I have the raspberry pi zero w becuz its got the camera connector on the right side.

'w' stands for wireless.


https://www.youtube.com/watch?v=TUz2mVtJVsM








The specs of it is here:
http://socialcompare.com/en/review/raspberry-pi-zero-wireless

        802.11 b/g/n wireless LAN
        Bluetooth(R) 4.1
        Bluetooth Low Energy (BLE)
        1GHz, single-core CPU
        512MB RAM
        Mini HDMI and USB On-The-Go ports
        Micro USB power
        HAT-compatible 40-pin header pins
        Composite video and reset headers
        CSI camera connector



if you want to use the Mini HDMI, usually you would get a HDMI to mini HDMI adapter.


so you need a micro SD card




Getting Started with the Raspberry Pi Zero Wireless
https://learn.sparkfun.com/tutorials/getting-started-with-the-raspberry-pi-zero-wireless/all
https://media.digikey.com/pdf/Data%20Sheets/Sparkfun%20PDFs/Getting_Started_with_RaspberryPiZeroWireless_Web.pdf







The Raspberry Pi Zero has a 32-bit ARMv6Z architecture with the Broadcom BCM2835 SoC found in the Model A and Model B+ Pis. Similarly, the CPU is a 1GHz single-core ARM1176JZF-S, similar to that found on the original Pis (but bumped up from 700MHz). It has 512MB shared RAM, and the 1.3 revision boards (those released since May 2016) also have the MIPI camera interface.

Equipped with a micro USB for power, and another for data only, the Pi Zero has mini HDMI-out and a microSD slot as expected. Stereo audio can be output via the GPIO. Although the GPIO pins are removed, the array — along with the Run and TV I/O — remain. This means that they can still be used, either by soldering, or manually adding GPIO pins (kits are available).

https://www.makeuseof.com/tag/raspberry-pi-board-guide/



so some of the videos I watched to better understand how the raspberry Pi work:

Raspberry Pi 4 Getting Started
https://www.youtube.com/watch?v=BpJCAafw2qE


so you need micro SD card





-       Raspberry Pi Zero Specs

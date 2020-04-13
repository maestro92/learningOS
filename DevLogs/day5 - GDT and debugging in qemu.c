###############################################################################
####################### x86 memory segmentation ###############################
###############################################################################

as mentioned in day 4, the intel 80286 introduced a 2nd version of segmentation in 1982 that added support for virtual memory and memory protection.
And GDT is how they accomplished the memory protection. 

The GDT is a data structure, starting with the 80286 in order to define the characteristics of the various memory areas. 
For example, 

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
######################## GDT Table ########################################
###########################################################################

So we actually have to define to things, the GDT descriptor and the GDT table itself.
details can be seen here 
https://wiki.osdev.org/Global_Descriptor_Table



Each GDT entry also defines whether or not the current segment that the processor is running in is for System use 
(Ring 0) or for Application use (Ring 3). There are other ring types, but they are not important. 
Major operating systems today only use Ring 0 and Ring 3.


one may ask, why is the gdt entry designed this way:
https://softwareengineering.stackexchange.com/questions/404412/why-is-the-data-for-an-x86-gdt-entry-designed-this-way

The short answer is, mainly for backwards compatibility. the x86 was originally 16-bit, then it had to evolve to work with 
larger registers.






so detailed explanation of the values can be found here:
https://wiki.osdev.org/GDT


The base is a 32 bit value containing the linear address where the segment begins 
The limit, 
apparently the processor_s interpretation of the limit dependsd on the setting of the G (granularity) bit 
For data segments, the processor_s interpretation of the limit depends also on the E-bit (expansion-direction bit) 
and the B-bit (big bit)

When G = 0, it means use bytes. the actual imit is the value of the 20-bit field as it appears in the descriptor. 
In this case, the limit may range from 0 to 0FFFFFH (220 - 1 or 1 megabyte). 

When G = 1, it means use 4KiB. the processor appends 12 low-order one-bits to the value in the limit field. 
In this case the actual limit may range from 0FFFH (212 - 1 or 4 kilobytes) to 0FFFFFFFFH (232 - 1 or 4 gigabytes).

https://stackoverflow.com/questions/26577692/what-exactly-does-the-granularity-bit-of-a-gdt-change-about-addressing-memory



also notice in the doc, the order is reveresd
where we first fill in data on the 2nd line (first filling in Base address then Segment Limit)
then the 1st line 

the link on the OS dev website has the correct order
https://wiki.osdev.org/Global_Descriptor_Table




for our descriptor table, 

the size is two bytes, the offset is 4 bytes 
so for size we just do 
        

                gdt_descirptor:
                    dw  gdt_end - gdt_start - 1 

                    dd  gdt_start 



so we wrote the stuff in 32bit-gdt.asm



###########################################################################
################################### Switching #############################
###########################################################################

links:
https://www.cs.bham.ac.uk/~exr/lectures/opsys/10_11/lectures/os-dev.pdf
(chapter 4.4)

https://wiki.osdev.org/Protected_Mode


so we have defined the GDT. 
we do a few things to do the switch 
here is an example: 

1.  Disable Interrupts 
2.  Load GDT desecriptor
3.  Set the first bit of a special CPU control register, cr0
4.  make far jump 

-   Then we make the actual swtich, by setting the first bit of a special CPU control register, cr0.


-   after the cr0 has been updated, the CPU is in protected mode. 

    we will want to call a far jump, which will force the CPU to flush the pipeline (complete 
    all of instructiosn currently in different stages of the pipeline)

    note that a near jump may not be enough to flush the pipeline (jmp start_protected_mode).
    so we want to think about where we walnd. Our current code segment (ie, cs) will not be valid 
    in protected mode, so we must update our cs register to the offset of our code segment descriptor of our GDT. 

    in our case its CODE_SEG which we calculated in our 32bit-gdt.asm. recall the first entry is 0x0, 
    the 2nd entry is 0x8. so that is what our code segment register should point to. 

    Note that by the very definition of a far jump, it will automatically cause the CPU to update our cs register 
    to the target segment. 

    to issue a far jump, as opposed to a near jump, we additionall provde the target segment 

                jmp <segment>:<address offset>

    so in our case, we just do 

                jmp CODE_SEG:start_protected_mode

    

-   full code below:
        

                cli
                lgdt [gdt_descirptor]
                mov eax, cr0 
                or eax, 0x1
                mov cr0, eax 
                jmp CODE_SEG:init_protected_mode


               



after we switched, we need to tell our code to be in 32 bit and we do a bunch of initalization for the protected mode 

-   the [bits 32] directive tells our assembler that from that point onwards, it should encode in 32-bit mode instructions.
    so now we are both 32 bit and protected mode 


-   we then should update all the other segment registers so they now point to our 32-bit data segment 
    (rather than the now invalid real mode segments) and update the position of our stack 

                [bits 32]
                init_protected_mode:
                   mov   ax, DATA_SEG       ; 0x10 points at the new data selector
                   mov   ds, ax
                   mov   ss, ax
                   mov   es, ax
                   mov   fs, ax
                   mov   gs, ax

                   mov ebp, 0x90000     ; update the stack again
                   mov esp, ebp         ;   


                   call start_protected_mode        


                [bits 32]
                start_protected_mode:

                    mov ebx, MSG_PROTECTED_MODE
                    call print_string_pm
                    jmp $


we use the [bits 32] directive to tell our assembler that from that point onwards, 
it should encode in 32-bit mode instructions. 

and then we start calling print_string_pm. 

Then you should be able to see "Loaded 32-bit protected mode" on the top left corner. 
If its too hard to see, you can change the color in the "print_string_pm" function.
in print_string_pm, we are doing WHITE_ON_BLACK. Consdier changing to some other color. 


#######################################################################
######################## Debugging ####################################
#######################################################################

-   gdb and qemu
so obviously, our current code is getting a bit complicated, and your code may not run fine, so we have to debug.
So to debug in qemu, this is what you have to do

https://wiki.osdev.org/Kernel_Debugging#Use_GDB_with_QEMU

                qemu -s -S <harddrive.img>

the '-s' option makes qemu listen on port tcp::1234, which you can connect to as localhost:1234 if you are on the same machine.
Qemu_s '-S' makes Qemu stop execution until you give the continue command. 

https://stackoverflow.com/questions/11408041/how-to-debug-the-linux-kernel-with-gdb-and-qemu
https://stackoverflow.com/questions/9865657/operating-system-debugger-for-ubuntu-11-10/10283339#10283339

now start another shell, and you can start GDB, then you can run 
                
                >> gdb 
                (gdb) target remote localhost:1234

and that will connect to qemu.
so when you run gdb and you run 
                
                (gdb) target remote localhost:1234

I was getting the error 
                
                "Remote 'g' packeet reply is too long:... "


https://wiki.osdev.org/QEMU_and_GDB_in_long_mode

turns out that this means qemu is running in long, which make sense cuz I am running qemu-x86-64
so I tried qemu-system-i386, so the command looks like 

                qemu-system-i386 -s -S [binary_img]

https://stackoverflow.com/questions/22534152/can-nasm-generate-debug-symbol-to-binary-file
and gdb is able to connection fine. 


now you can see that qemu_s execution is paused, but you can really debug cuz gdb doesnt have the symbol-file 

so now the question is, how do you generate the symbol-file from nasm and our hardware img 
recall previously, the way we were compiling is 

                nasm -f bin main.asm -o main.img 

which is pure binary. 

so turns out the following links has the solution:
https://stackoverflow.com/questions/32955887/how-to-disassemble-16-bit-x86-boot-sector-code-in-gdb-with-x-i-pc-it-gets-tr

what you want to do is to compile it as elf, and use objcopy to get the code segment out. 

so lets do that, we first run 

                nasm -f elf32 -g3 -F dwarf main.asm -o main.o

so we see a bunch of compile flags, you can see use "nasm -help" to see what the compile flags mean 

-   "-f" is the output format 
-f bin, which we did previously is pure binary 
-f elf32, is 32 bit elf format (executable and linkable format)

-   "-g3" is 
generating debug file

-   "-F dwarf"
 -F format is selecting a debugging format. you can see the list of supported debug file format for an output file by doing 
 nasm -f <format> -y 

for example, lets say we run 

                $ nasm -f bin -y

                valid debug formats for 'bin' output format are ('*' denotes default):
                    null      Null debug format

you can see, we cant generate any debug formats for pure binary file 

lets say we run 

                $ nasm -f elf32 -y

                valid debug formats for 'elf32' output format are ('*' denotes default):
                    dwarf     ELF32 (i386) dwarf debug format for Linux/Unix
                    stabs     ELF32 (i386) stabs debug format for Linux/Unix

and we can see we have dwarf file format 


-   elf and org directive 
so the next error you will see is that you can generate an elf file with nasm because of the [org] directive.
apparently [org] doesnt make sense with elf files cuz linker can relocate an elf file to anywhere it wants.
so the solution is to actually get rid of it. See the link above 

after you remove or comment out the [org 0x7c00] line, you want to use the linker to offset the code 


                $ ld -Ttext=0x7c00 -melf_i386 main.o -o main.elf

so if you look at what does "-Ttext" does 

                $ ld -help | grep Ttext
                  -Ttext ADDRESS              Set address of .text section
                  -Ttext-segment ADDRESS      Set address of text segment


essentially we are offseting the "text segment", which will do equivalently what [org 0x7c00] does. 

the compile flag for "-melf_i386", so specifying the target we are linking for. 



so after linking it and offsetting our elf file, we want to copy out the text segment, using objcopy 

                $ objcopy -O binary main.elf main.img

objcopy is a tool under the GNU Binary Utilities.
quoting this link:
https://sourceware.org/binutils/docs/binutils/objcopy.html

        "It can write the destination object file in a format different from that of the source object file"



then we finally start qemu:
                
                -system-i386 -hda main.img -S -s 


regarding the -hda compile flag, it means:

                $ qemu-system-i386 -help | grep hda
                -hda/-hdb file  use 'file' as IDE hard disk 0/1 image



-   full procedure looks like: 

                $ nasm -f elf32 -g3 -F dwarf main.asm -o main.o
                $ ld -Ttext=0x7c00 -melf_i386 main.o -o main.elf
                $ objcopy -O binary main.elf main.img
                $ qemu-system-i386 -hda main.img -S -s 


start another shell 

                $ gdb main.elf 
                (gdb) 'target remote localhost:1234' 
                (gdb) 'set architecture i8086' 
                (gdb) 'layout src' 
                (gdb) 'layout regs' 
                (gdb) 'break main' 
                (gdb) 'continue'


then you can start calling "stepi" to step through it



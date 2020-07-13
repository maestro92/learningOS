Chapter 5.2.1
https://www.cs.bham.ac.uk/~exr/lectures/opsys/10_11/lectures/os-dev.pdf



so we will finally write our kernel, which will be written in C. 
lets first write a super simple kernel 

                kernel.c

                int kernel_main()
                {
                    unsigned char* vga = (unsigned char*) 0xb8000;
                   
                    unsigned char* name = "Martin OS";
                    char* cur = name;
                    int index = 0;
                    while (*cur != '\0')
                    {
                        vga[index] = *cur;
                        vga[index+1] = 0x09;

                        cur++;
                        index += 2;
                    }

                   for(;;); //make sure our kernel never stops, with an infinite loop
                }


we will also write a bootsector now, that will bostrap our kernel from the disk. 
The idea is that we can actually graft our boots sector and kernel of an OS together 
into a kernel image. 

So imagine we compile our kernel code into binary 

                $i686-elf-gcc -m32 -ffreestanding -nostdlib -lgcc -c kernel.c -o build/kernel.o
                $ld -Ttext=0x1000 --oformat binary -melf_i386 build/kernel.o -o build/kernel.img 

and we get this kernel.bin file. (more will be explained later)

note that we are using the cross compiler that we built in day 1. Which is why we are doing i686-elf-gcc instead of gcc.

and then we compile our bootsector code, and we compile it into bootsect.bin 


we will concatenate the two together through the "cat" command 

                cat bootsect.bin kernel.bin > os-image 

essentially what the os-image will look like in binary is 
                
                os-image
                 _______________________
                |                       |                       
                |     bootsect.bin      |
                |       512 bytes       |
                |_______________________|
                |                       |                       
                |                       |
                |                       |
                |     kernel.bin        | 
                |       (?? bytes)      |                       
                |                       |
                |                       |
                |_______________________|

we know that the boot_sect.bin is 512 bytes, but kernel.bin can be however long it wants to be.


Recall that in day2 when we talked about bootsector code, it usually does a chained loading becuz you cant fit 
the entire kernel in the first 512 bytes. You have to put the kernel code in later parts of your disk. We are gonna do exactly that. 
We are putting our kernel right after our bootsector code.

so we have the following, 
the idea is that 
-   we first call "load_kernel", to load into 0x1000
    

-   then we switch into protected mode and call 

                32bit-main.asm


                KERNEL_OFFSET equ 0x1000    ; memory offset to which we will load our kernel

                boot_sector_start:

                    mov [BOOT_DRIVE], dl    ;   when we boot, BIOS stores the boot dirve in the dl register
                                            ;   we want to extract that number and put it into [BOOT_DRIVE]

                    mov bp, 0x9000          ; set the stack safely away from us
                    mov sp, bp

                    mov bx, MSG_REAL_MODE
                    call print

    ----------->    call load_kernel    

                switching:
                    call switch_to_protected_mode
                    jmp $   ;   this actually never gets executed 


                %include "real_mode_print_string.asm"
                %include "disk_load.asm"
                %include "32bit-gdt.asm"
                %include "32bit-print.asm"
                %include "32bit-switch.asm"

                [bits 16]
    ------->    load_kernel:
                    mov bx, MSG_LOAD_KERNEL
                    call print 

                    mov bx, KERNEL_OFFSET
                    mov dh, 15
                    mov dl, [BOOT_DRIVE]    ;   drive number (0=A:, 1=2nd floppy, 80h=drive 0, 81h=drive 1)
                    call disk_load
                    
                    ret 


                [bits 32]
                begin_protected_mode:

                    mov ebx, MSG_PROTECTED_MODE
                    call print_string_pm
    ----------->    call KERNEL_OFFSET
                    jmp $


notice in the load_kernel routine, we are loading 15 segments (512 * 15 bytes) from the boot disk. You may wonder 
why do we want to do that when our kernel image was much smaller than this?
The reason is becuz it doesnt hurt to read those additional sectors from the disk. It actually may hurt if we didnt 
read enough sectors at this stage. 





visually this is what happening

1.  BIOS first load our bootsector code at 0x7c00, we run our boot_sect.bin


        fake hard drive                      ___________________    <--------- 0x00000000 
         os.image                           |                   |
     _______________________                |                   |
    |                       |               |                   |
    |     boot_sect.bin     |               |                   |   
    |       512 bytes       |------         |                   |
    |_______________________|     |         |                   |
    |                       |     |         |                   |
    |                       |     |         |                   |
    |                       |     |         |                   |
    |     kernel.bin        |     |         |                   |
    |       (?? bytes)      |     |         |                   |
    |                       |     |         |___________________|   <--------- 0x00007C00 (31 kb)         
    |                       |     |         |                   |
    |_______________________|     ------->  |   bootsector      |
                                            |                   |
                                            |                   |
                                            |___________________|   <--------- 0x00008000 (32 kb)
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



2.  we call load_kernel, which reads from our hard drive (disc image in this case);, and we load our kernel code to 0x1000
    and we run our kernel code, which is the main function in kernel.c


        fake hard drive                      ___________________    <--------- 0x00000000 
         os.image                           |                   |
     _______________________                |                   |
    |                       |               |___________________|
    |     boot_sect.bin     |               |                   |   <--------- 0x00001000 (4 kb)   
    |       512 bytes       |     --------> |     kernel.c      |
    |_______________________|     |         |                   |
    |                       |     |         |___________________|
    |                       |     |         |                   |
    |                       |------         |                   |
    |     kernel.bin        |               |                   |
    |       (?? bytes)      |               |                   |
    |                       |               |___________________|   <--------- 0x00007C00 (31 kb)         
    |                       |               |                   |
    |_______________________|               |   bootsector      |
                                            |                   |
                                            |                   |
                                            |___________________|   <--------- 0x00008000 (32 kb)
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




so now lets go in detail we will make our os.image 
to compile our bootsector code, we do 




To compile kernel code we do:

                $ i686-elf-gcc -m32 -ffreestanding -nostdlib -lgcc -c kernel.c -o build/kernel.o
                $ ld -Ttext=0x1000 --oformat binary -melf_i386 build/kernel.o -o build/kernel.img 

so there are a few details we need to pay attention.

-   first is, we need to use the cross compiler, "i686-elf-gcc", not your platforms compiler, probably "gcc"
I figured out this problem from this link: https://forum.osdev.org/viewtopic.php?f=1&t=25796

if you do use gcc, like below:

                $ gcc -m32 -ffreestanding -nostdlib -lgcc -c kernel.c -o build/kernel.o
                $ ld -Ttext=0x1000 --oformat binary -melf_i386 build/kernel.o -o build/kernel.img 

you will get the following error                

        
        "ld: warning: cannot find entry symbol _start; defaulting to 0000000000001000
        ld: build/kernel.o:kernel.c:(.text+0xa): undefined reference to '___main'"

-   another thing is that you want ot pass in "-ffreestanding -nostdlib -lgcc" compile flags. 
I dont really know why, but the following link says to use these three flags
https://stackoverflow.com/questions/32164478/when-using-ld-to-link-undefined-reference-to-main/32165150#32165150

From looking at what each compile flags does, it says:
https://gcc.gnu.org/onlinedocs/gcc/C-Dialect-Options.html#C-Dialect-Options

-ffreestanding
            
        "Assert that compilation targets a freestanding environment. This implies -fno-builtin. 
        A freestanding environment is one in which the standard library may not exist, and program startup may not necessarily be at main. 
        The most obvious example is an OS kernel. This is equivalent to -fno-hosted."

-nostdlib
        
        "Do not use the standard system startup files or libraries when linking. 
        No startup files and only the libraries you specify are passed to the linker, and options specifying linkage of the system libraries, 
        such as -static-libgcc or -shared-libgcc, are ignored. The compiler may generate calls to memcmp, memset, memcpy and memmove. 
        These entries are usually resolved by entries in libc. These entry points should be supplied through some other mechanism when this option is specified.
        One of the standard libraries bypassed by -nostdlib and -nodefaultlibs is libgcc.a, 
        a library of internal subroutines which GCC uses to overcome shortcomings of particular machines, or special needs for some languages. 
        (See Interfacing to GCC Output in GNU Compiler Collection (GCC) Internals, for more discussion of libgcc.a.) 
        In most cases, you need libgcc.a even when you want to avoid other standard libraries. 
        In other words, when you specify -nostdlib or -nodefaultlibs you should usually specify -lgcc as well. 
        This ensures that you have no unresolved references to internal GCC library subroutines. 
        (An example of such an internal subroutine is __main, used to ensure C++ constructors are called; see collect2 in GNU Compiler Collection (GCC) Internals.)"

-lgcc
        goes with nostdlib


so I just added these three compile flags


-   When linking our kernel.o, we want our kernel to run at 0x1000. Hence we do -Ttext 0x1000.
[I suppose here we dont have any memory dependent code, but if we do, which is why we want to do -Ttext 0x1000
    otherwise, why do we need to do this?]

                

So the full compile procedure is in the compile_martin_os.sh script. I guess you can also write it into a makefile

                compile_martin_os.sh

                # compiling the bootsector code
                nasm -f elf32 -g3 -F dwarf 32bit-main.asm -o build/bootsect.o
                ld -Ttext=0x7c00 -melf_i386 build/bootsect.o -o build/bootsect.elf
                objcopy -O binary build/bootsect.elf build/bootsect.img

                # compiling the kernel
                i686-elf-gcc -m32 -ffreestanding -nostdlib -lgcc -c kernel.c -o build/kernel.o
                ld -Ttext=0x1000 --oformat binary -melf_i386 build/kernel.o -o build/kernel.img 

                # build the os-image
                cat build/bootsect.img build/kernel.img > build/martinos.img


#######################################################################################
########################## Bug with Disk Load #########################################
#######################################################################################                

So recallin our code, we initially had 


                KERNEL_OFFSET equ 0x1000    ; memory offset to which we will load our kernel

                boot_sector_start:

                    mov [BOOT_DRIVE], dl    ;   when we boot, BIOS stores the boot dirve in the dl register
                                            ;   we want to extract that number and put it into [BOOT_DRIVE]

                    mov bp, 0x9000          ; set the stack safely away from us
                    mov sp, bp

                    mov bx, MSG_REAL_MODE
                    call print

                    call load_kernel    

                switching:
                    call switch_to_protected_mode
                    jmp $   ;   this actually never gets executed 


                %include "real_mode_print_string.asm"
                %include "disk_load.asm"
                %include "32bit-gdt.asm"
                %include "32bit-print.asm"
                %include "32bit-switch.asm"

                [bits 16]
                load_kernel:
                    mov bx, MSG_LOAD_KERNEL
                    call print 

                    mov bx, KERNEL_OFFSET
    ----------->    mov dh, 15
                    mov dl, [BOOT_DRIVE]    ;   drive number (0=A:, 1=2nd floppy, 80h=drive 0, 81h=drive 1)
                    call disk_load
                    
                    ret 


                [bits 32]
                begin_protected_mode:

                    mov ebx, MSG_PROTECTED_MODE
                    call print_string_pm
                    call KERNEL_OFFSET
                    jmp $


where we were loading 15 sections. When I experimented with it, I was getting "Disk read error!" messages. 
So when I changed it to 2 sections, i was still getting the error. Then eventually I changed it to 1 section, it works fine.
I think it has to do with the length of your os-image. So when I opened up my os-image file in sublime, this is the output: 

                8816 297d bd00 9089 ecbb 2a7d e80b 00e8
                1a00 e8f0 00e8 be00 ebfe 608a 073c 0074
                09b4 0ecd 1083 c301 ebf1 61c3 60b4 0eb0
                0acd 10b0 0dcd 1061 c360 b900 0083 f904
                741c 89d0 83e0 0f04 303c 397e 0204 07bb
                6b7c 29cb 8807 c1ca 0483 c101 ebdf bb66
                7ce8 b6ff 61c3 3078 3030 3030 0060 52b4
                0288 f0b5 00b6 00b1 02cd 1372 075a 38c6
                7502 61c3 bb8c 7ce8 90ff ebfe 4469 736b
                2072 6561 6420 6572 726f 7221 0000 0000
                0000 0000 00ff ff00 0000 9acf 00ff ff00
                0000 92cf 0017 009d 7c00 0060 ba00 800b
                008a 03b4 0f3c 0074 0b66 8902 83c3 0183
                c202 ebed 61c3 fa0f 0116 b57c 0f20 c066
                83c8 010f 22c0 eaeb 7c08 0066 b810 008e
                d88e d08e c08e e08e e8bd 0000 0900 89ec
                e813 0000 00bb 637d e80f ffbb 0010 b601
                8a16 297d e856 ffc3 bb46 7d00 00e8 99ff
                ffff e8d9 92ff ffeb fe00 5374 6172 7465
                6420 696e 2031 362d 6269 7420 7265 616c
                206d 6f64 6500 4c6f 6164 6564 2033 322d
                6269 7420 7072 6f74 6563 7465 6420 6d6f
                6465 004c 6f61 6469 6e67 206b 6572 6e65
                6c20 696e 746f 206d 656d 6f72 792e 0000
                0000 0000 0000 0000 0000 0000 0000 0000
                0000 0000 0000 0000 0000 0000 0000 0000
                0000 0000 0000 0000 0000 0000 0000 0000
                0000 0000 0000 0000 0000 0000 0000 0000
                0000 0000 0000 0000 0000 0000 0000 0000
                0000 0000 0000 0000 0000 0000 0000 0000
                0000 0000 0000 0000 0000 0000 0000 0000
                0000 0000 0000 0000 0000 0000 0000 55aa
                5589 e583 ec10 c745 f400 800b 00c7 45f0
                5710 0000 8b45 f089 45fc c745 f800 0000
                00eb 288b 45fc 0fb6 088b 55f8 8b45 f401
                d089 ca88 108b 45f8 8d50 018b 45f4 01d0
                c600 0983 45fc 0183 45f8 028b 45fc 0fb6
                0084 c075 ceeb fe4d 6172 7469 6e20 4f53
                0000 0000 1400 0000 0000 0000 017a 5200
                017c 0801 1b0c 0404 8801 0000 1800 0000
                1c00 0000 7cff ffff 5700 0000 0041 0e08
                8502 420d 0500 0000 

each row is 16 bytes, we got 41 rows + 8 on the 42nd row 
so a total of 664 bytes. So this makes sense cuz of you are loading two more section after the MBR, that means you are loading 
1024 bytes, which I clearly dont have. So the fact that disk_load routine only works correctly with loading 1 section (at this point), makes sense. 

I guess we will just have to change the number manually as the lenght of our file becomes longer?


###########################################################
########### Explicit kernel Entry point ###################
###########################################################

so one problem with our kernel.c file is that it assmumes main is the first function of the file. 
the fact that we are calling 

                32bit-main.asm


                KERNEL_OFFSET equ 0x1000    ; memory offset to which we will load our kernel

                boot_sector_start:
                    ...
                    ...

                switching:
                    ...
                    ...

                %include "real_mode_print_string.asm"
                %include "disk_load.asm"
                %include "32bit-gdt.asm"
                %include "32bit-print.asm"
                %include "32bit-switch.asm"

                [bits 16]
                load_kernel:
                    ...
                    ...

                [bits 32]
                begin_protected_mode:

                    mov ebx, MSG_PROTECTED_MODE
                    call print_string_pm
    ----------->    call KERNEL_OFFSET
                    jmp $


call KERNEL_OFFSET, this assumes our main function is the first instruction at that address. This assumption clearly 
falls apart if your kernel.c file looks like:


                kernel.c 

                void foo()
                {
                    ...
                }

                void kernel_main()
                {
                    ...
                }




so one solution to this problem is to write an intermediate assembly snippet that explicitly calls into kernel_main
So now we will add an explicity kernel_entry.asm file. 

                kernel_entry.asm

                [bits 32]
                [extern kernel_main]    ; Define calling point. Must have same name as kernel.c 'kernel_main' function
                call kernel_main        ; Calls the C function. The linker will know where it is placed in memory
                jmp $


so in the assembly code here, we are using extern 
accordingly to the official nasm specs, it says:

        "EXTERN is similar to the MASM directive EXTRN and the C keyword extern: 
        it is used to declare a symbol which is not defined anywhere in the module being assembled, 
        but is assumed to be defined in some other module and needs to be referred to by this one. 
        Not every object-file format can support external variables: the bin format cannot."

as a side note, there is also the "global" directive






so here we declared main as an external symbol, and that will be referring to the kernel_main function in our kernel.c file

Update:
supposedly this extern trick is supposed to work, but I couldnt get it to work. So thing I did is that 
I have to ensure the kernel_main is the first function of my kernel.c file. So I have the following setup. 

                kernel_entry.asm 

                [bits 32]
                [extern _start] ; Define calling point. Must have same name as kernel.c 'main' function
                call _start ; Calls the C function. The linker will know where it is placed in memory
                jmp $


and our kernel.c file looks like: 

                kernel.c

                int kernel_main();

                int _start()
                {
                   kernel_main();
                }

                #include "../drivers/screen.c"

                int kernel_main()
                {
                   unsigned char* vga = (unsigned char*) 0xb8000;
                   
                   unsigned char* name = "Martin OS22";
                   char* cur = name;
                   int index = 0;
                   while (*cur != '\0')
                   {
                      vga[index] = *cur;
                      vga[index+1] = 0x09;

                      cur++;
                      index += 2;
                   }
                }


this apparently works fine.

##############################################################################################
################################# Debugging the kernel #######################################
##############################################################################################


so now when we compile we have to add this in and link it with our kernel.c


                build.sh

                # compiling the bootsector code
                nasm -f elf32 -g3 -F dwarf 32bit-main.asm -o build/bootsect.o
                ld -Ttext=0x7c00 -melf_i386 build/bootsect.o -o build/bootsect.elf
                objcopy -O binary build/bootsect.elf build/bootsect.img

                # compiling the kernel_entry code
                nasm -f elf32 -g3 -F dwarf kernel_entry.asm -o build/kernel_entry.o

                # compiling the kernel
                i686-elf-gcc -m32 -ffreestanding -nostdlib -lgcc -c kernel.c -o build/kernel.o                
                ld -Ttext=0x1000 --oformat binary -melf_i386 build/kernel.o build/kernel_entry.o -o build/kernel.img 

                # build the os-image
                cat build/bootsect.img build/kernel.img > build/martinos.img






So now if you look at it we can debug the kernel.elf or the bootsector.elf.
so if you want to debug the kernel, you have to make an elf for for that. So it would look like:


                build.sh

                # compiling the bootsector code
                nasm -f elf32 -g3 -F dwarf 32bit-main.asm -o build/bootsect.o
                ld -Ttext=0x7c00 -melf_i386 build/bootsect.o -o build/bootsect.elf
                objcopy -O binary build/bootsect.elf build/bootsect.img

                # compiling the kernel_entry code
                nasm -f elf32 -g3 -F dwarf kernel_entry.asm -o build/kernel_entry.o

                # compiling the kernel
                i686-elf-gcc -m32 -ffreestanding -nostdlib -lgcc -c -g3 kernel.c -o build/kernel.o      <------------          
                ld -Ttext=0x1000 -melf_i386 build/kernel.o build/kernel_entry.o -o build/kernel.elf     <------------
                objcopy -O binary build/kernel.elf build/kernel.img

                # build the os-image
                cat build/bootsect.img build/kernel.img > build/martinos.img



now if we want to debug the kernel, you have to compile in -g mode when you compile kernel.c
and just load in the kernel.elf file when you run gdb


                $ gdb build/kernel.elf
                (gdb) 'target remote localhost:1234' 
                (gdb) 'set architecture i8086' 
                (gdb) 'layout src' 
                (gdb) 'layout regs' 
                (gdb) 'break main' 
                (gdb) 'continue'





#####################################################################
########################### Makefile ################################
#####################################################################

so if you are even mroe tired of typing, I made a makefile that automates everything 

                all:
                    # compiling the bootsector code
                    nasm -f elf32 -g3 -F dwarf boot/32bit-main.asm -o build/bootsect.o
                    ld -Ttext=0x7c00 -melf_i386 build/bootsect.o -o build/bootsect.elf
                    objcopy -O binary build/bootsect.elf build/bootsect.img

                    # compiling the kernel_entry code
                    nasm -f elf32 -g3 -F dwarf boot/kernel_entry.asm -o build/kernel_entry.o

                    # compiling the kernel
                    i686-elf-gcc -m32 -ffreestanding -nostdlib -lgcc -g -c kernel/kernel.c -o build/kernel.o                
                    ld -Ttext=0x1000 -melf_i386 build/kernel.o build/kernel_entry.o -o build/kernel.elf 
                    objcopy -O binary build/kernel.elf build/kernel.img

                    # build the os-image
                    cat build/bootsect.img build/kernel.img > build/martinos.img

                run:
                    qemu-system-i386 -hda build/martinos.img

                run_debug:
                    qemu-system-i386 -hda build/martinos.img -s -S

                debug_boot:
                    gdb -x gdb_boot

                debug_kernel:
                    gdb -x gdb_kernel

                clean:
                    rm -rf build/*.elf build/*.img build/*.o                                                                               
*/


so if you want to compile your script just run make, since make runs the first target. or you can specify make all 

when you want to emulate, just run make run 

to debug, run the "make debug_kernel"

notice the command is: "gdb -x gdb_boot"

what is happening here is that notice that once you start a gdb session, you have to run a shit ton of commands inside gdb, 
apparently you can save a bunch of commands a separate file, and gdb -x allows you to run all these commands in a file. 
so I actually have a separate file called gdb_boot and gdb_kernel that looks like below:
                
                gdb_boot 

                target remote localhost:1234
                symbol-file build/bootsect.elf

and 

                gdb_kernel

                target remote localhost:1234
                symbol-file build/kernel.elf

for more info you can see the following stackoverflow link 
https://stackoverflow.com/questions/14226563/how-to-read-and-execute-gdb-commands-from-a-file



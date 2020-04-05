Chapter 5.2.1
https://www.cs.bham.ac.uk/~exr/lectures/opsys/10_11/lectures/os-dev.pdf



so we will finally write our kernel, which will be written in C. 
lets first write a super simple kernel 

                kernel.c

                void main()
                {
                    char* video_memory = (char*) 0xb8000;

                    *video_memory = 'X';
                }



we will also write a bootsector now, that will bostrap our kernel from the disk. 
The idea is that we can actually graft our boots sector and kernel of an OS together 
into a kernel image. 

So imagine we compile our kernel code into binary 

                $gcc -ffreestanding -c kernel.c -o kernel.o
                $ld -o kernel.bin -Ttext 0x1000 kernel.o --oformat binary

and we get this kernel.bin file. (more will be explained later)

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


The kernel code we do:

                $gcc -ffreestanding -c kernel.c -o kernel.o
                $ld -o kernel.bin -Ttext 0x1000 kernel.o 

we want our kernel to run at 0x1000. Hence we do -Ttext 0x1000.
[I suppose here we dont have any memory dependent code, but if we do, which is why we want to do -Ttext 0x1000
    otherwise, why do we need to do this?]

                












so in the assembly code here, we are using extern 
accordingly to the official nasm specs, it says:

        "EXTERN is similar to the MASM directive EXTRN and the C keyword extern: 
        EXTERN is similar to the MASM directive EXTRN and the C keyword extern: 
        it is used to declare a symbol which is not defined anywhere in the module being assembled, 
        but is assumed to be defined in some other module and needs to be referred to by this one. 
        Not every object-file format can support external variables: the bin format cannot."


so here we declared main as an external symbol, and that will be referring to the main function in our kernel.c file

                kernel_entry.asm 

                [bits 32]
                [extern main]
                call main
                jmp $



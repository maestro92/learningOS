http://www.jamesmolloy.co.uk/tutorial_html/index.html

So the tools you need are GCC, ld, NASM and GNU Make. 


-   NASM
https://en.wikipedia.org/wiki/Netwide_Assembler

the Netwise Assembler, is an assembler and disassembler for the intel x86 architecture. It can be used to write 16-bit, 32-bit, and 64-bit programs 
NASM is considered to be one of the most popular assemblers for Linux. 


lets first take a look at the boot.s file 
recall from day 2, we have to first write a bootloader header
in NASM, the command you can use is the 'dd' command. 

we first define 5 macros
                
                MBOOT_PAGE_ALIGN    equ 1<<0   
                MBOOT_MEM_INFO      equ 1<<1    
                MBOOT_HEADER_MAGIC  equ 0x1BADB002 
                MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
                MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)





then we actually add the constants to our header 

                dd MBOOT_HEADER_MAGIC
                dd MBOOT_HEADER_FLAGS
                dd MBOOT_CHECKSUM
                dd mboot
                dd code
                dd bss
                dd end
                dd start


so lets look what each constants do 



-   MBOOT_HEADER_MAGIC  equ 0x1BADB002 
recall in day2_s article, we mentioned that 0x1BADB002 is a magic number. This identifies the kernel as multiboot-compatible.
https://wiki.osdev.org/Multiboot


-   MBOOT_PAGE_ALIGN    equ 1<<0   
    MBOOT_MEM_INFO      equ 1<<1  
    MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
a field of flags. We ask for GRUB to page-align all kernel sections with. (MBOOT_PAGE_ALIGN)
then we ask for GRUB to give us some memory information (MBOOT_MEM_INFO). Note that some tutorials also use MBOOT_AOUT_KLUDGE.
As we are using the ELF file format, this hack is not necessary, and assking it stops GRUB giving you your symbol table when you boot up. 


-   MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)
This field is defined such that when the magic number, the flags and this are added together, the total must be zero. It is for error checking. 


-   mboot 
The address of the structure that we are currently writing. GRUB uses this to tell if we are expecting to be relocated. 

-   code, bss, end, start 
These symbols are all defined by the linker. We use them to tell GRUB where the different sections of our kernel can be located. 

so on bootup, GRUB will load a pointer to another information structure into the EBX register. This can be used to query the environment 
GRUB set up for us. 



regarding "so on bootup, GRUB will load a pointer to another information structure into the EBX register. "
This is apaprently mentioned in the grub specs:
https://www.gnu.org/software/grub/manual/multiboot/multiboot.html

it says 

        "Upon entry to the OS, the EBX register contains the physical address of a Multiboot information data structure,
        through which the boot loader communicates information to the OS. The OS can user or ignore any parts of the structure as it 
        chooses; all information passed by the boot loader is advisory only."





back to our code, the only code that actually happens in here is 

                push ebx 
                cli 
                call main 
                jmp $

so immediately on bootup, this assembly snippet tells the CPU to push the contents of EBX onto the stack 
(remember that EBX now contains a pointer to the multiboot information structure), disable interrupts (CLI),
call our 'main' C function (which we havent defined yet), then enter an infinite loop. 






#######################################################################################
############################ main(); function #########################################
#######################################################################################

now lets add our main function. 

                    
                    int main(struct multiboo* mboot_ptr)
                    {
                        return 0xDEADBABA;
                    }

as you can see here, it takes a parameter, which is the pointer to the multiboot struct. 



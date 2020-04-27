So now we got an idea of what Interrupts are and how IDT work, we now want to code it up in our kernel. 
we first code how how we want to define idt_entry

                typedef struct {
                    unsigned short offset_lower;
                    unsigned short segment_selector;
                    unsigned char zeros;
                    unsigned char flags;
                    unsigned short offset_upper;
                } idt_entry;

                struct idt_entry IDT[256];

the structure of each entry is in many tutorials. You can look at osdev 

https://wiki.osdev.org/Interrupts_tutorial
or even intel manual
https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.html
chapter 6.10

and obviously we have 256 of those. 



in case you are confused with our typedef works, recall that in C, you would usually do something like 
                
                typedef unsigned char BYTE;

or 
                
                typedef unsigned char byte;

you are literally defining a class, or an alias. I like to think of it as a #define



then of course we want to define the struct for the idt_descriptor. This is the thing that we have to load 
into the IDTR register. Quoting the intel manual:

        "the processor locates the IDT using the IDTR register. This register holds both a 32-bit base address 
        and 16-bit limit for the IDT"

        "The LIDT instruction loadsd the IDTR register with the base address and limit held in a memory operand"

so our struct looks like:
                
                typedef struct {
                    unsigned short limit; 
                    unsigned int address;
                } idt_descriptor;

in the intel spec you can see an image, where the lower 0~15 bits is the limit 
and the upper 16 ~ 47 bits is the base address. Therefore the limit comes first, then the address 

now that we want to actually setup our IDT. Now we want to fill up entries in our "struct idt_entry IDT[256];" table

-   side note:
apparently struct in C is different from struct in C++. struct in c is a keyword where you have to use everywhere.
this is unlike C++. 

so if you want to do c++ stype declaration, you can do 

                
                struct idt_entry_struct {
                    unsigned short offset_lower;
                    unsigned short segment_selector;
                    unsigned char zeros;
                    unsigned char flags;
                    unsigned short offset_upper;
                };

                typedef struct idt_entry_struct idt_entry;
                idt_entry IDT[256];


                struct idt_descriptor_struct{
                    unsigned short limit; 
                    unsigned int address;
                };

                typedef struct idt_descriptor_struct idt_descriptor;



so lets us write a function that acutally fills up an entry in our idt 
for this, you can refer to https://wiki.osdev.org/Interrupts_tutorial
            
                idt.c

                void initIDT()
                {
                    idt_descriptor.limit = sizeof(idt_entry) * 256 - 1;
                    idt_descriptor.address = (unsigned int*)(&IDT);


                }



so now looks at the details of how we setup the idt_entry,

                typedef struct {
                    unsigned short offset_lower;
                    unsigned short segment_selector;
                    unsigned char zeros;
                    unsigned char flags;
                    unsigned short offset_upper;
                } idt_entry;


So in the intel manual it says: 
(https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.html)

        The IDT may contain 3 kinds of descriptor 
        -   Task gates
        -   Interrupt gates
        -   Trap gates



you can think of it as 
-   Task handler
-   Interrupt handler 
-   Trap handler 


according to this link:
(http://littleosbook.github.io/#creating-an-entry-in-the-idt)

The task handlers (the 1st type); use functionality specific to the Intel version of x86,

The only difference between an interrupt handler and a trap handler is that the interrupt handler disables interrupts, 
which means you cannot get an interrupt while at the same time handling an interrupt. 
In this book, we will use trap handlers and disable interrupts manually when we need to.


so we know that IDT has 256 entries. According to the Intel amanual again:
        
        "Vector numbers in the range 0 through 31 are reserved by the
        Intel 64 and IA-32 architectures for architecture-defined exceptions and interrupts.

        Vector numbers in the range 32 to 255 are designated as user-defined interrupts and are not reserved by the Intel
        64 and IA-32 architecture. "



so here, we will setup interrupt handlers and trap handlers.


we first want to setup interrupt handlers. (the 2nd type);
So if you look at the intel manual, it has different format definitions for each of the 
Task Gate, Interrupt Gate and Trap Gate type 


and for the Interrupt Gate type, we have 

                                    Interrupt Gate 

                -----------------------------------------------------
                |   Offset 31 ... 16            P PDL 0D110 000     |    
                |                                                   |
                -----------------------------------------------------    
                |   Segment Selector            Offset 15 ... 0     |
                |                                                   |
                -----------------------------------------------------

so for explanations: 

-   the offset is a pointer to code (preferably an assembly code label).

-   the "segment_selector" is 0x08, cuz that is the kernel code sgement that we setup in our gdt 
"flags" is gonna be 0x8E


the flags is 

    Present       Descriptor Privilege Level      0 D 1 1 0  (size of gate) 

    1                                             0 1 1 1 0

DPL is gonna be a Ring value (0 to 3). Kernel ring level is 0

D is gonna be 1, since our gate are 32 bits in size, meaning the address offset to our handler 
are 32 bits in size. Hence D is 1 


so for most of our handlers, we will have 


    Present       Descriptor Privilege Level      0 D 1 1 0  (size of gate) 

    1                     00                       0 1 1 1 0

which gives you 10001110, which is 0x8E in hex


##########################################################################
######################## The Interrupt Handlers ##########################
##########################################################################

so now we have finished defining the idt_entry class, we now need to concern ourselves with the actual 
interrupt handlers. Interrupt handlers are also called interrupt service routines. 

        
        "so the interrupt service routines are used to save the current processor state and set up the appropriate segment registers
        needed for the kernel mode before the kernel_s C-level interrupt handler is called. 

        http://www.osdever.net/bkerndev/Docs/isrs.htm
        "

as mentioned, the idea is that the interrupt handlers will have to save all the register of the previous task. we will have to 
do that ourselves, which means we will essentially have to call calling "pusha and popa" instruction. 
this will require us having to write some assembly code, in our interrupt handler function. 
the general structure will look something like 

                file1.c 

                void initIDT()
                {
                    setup_idt(xxx, (u32int)interrupt_handler);
                }

                extern void interrupt_handler();

we declare interrupt_handler, but since we need to add some assembly code, the actual definition will exist in a an assembly file 
so then in our 2nd assembly file, we have: 

                file2.asm 
  
                global interrupt_handler

                interrupt_handler: 

                    pusha 

                    .... my interrupt handler logic ....

                    popa 
                    ret 



the .... my interrupt_handler logic .... can be in c, so you can have something like: 

                
                interrupt_handler: 

                    pusha 

                    call myC_level_interrupt_handler

                    popa 
                    ret 



another thing about this interrupt_handler routine is that we actually cant use ret,
as mentioned in the following few links 
http://www.logix.cz/michal/doc/i386/chp09-06.htm
https://wiki.osdev.org/Interrupt_Service_Routines
http://www.jamesmolloy.co.uk/tutorial_html/4.-The%20GDT%20and%20IDT.html


this link (http://www.logix.cz/michal/doc/i386/chp09-06.htm)
actually mentions that 
        
        "The 80386 invokes an interrupt or exception handling procedure 
        in much the same manner as it CALLs a procedure; the differences are explained in the following sections."

-   Just as with a control transfer due to a CALL instruction, a control transfer to an interrupt or exception handling procedure uses the stack 
to store the information needed for returning to the original procedure. 
an interrupt pushes the EFLAGS register onto the stack before the pointer to the interrupted instruction.

also note that certain types of exceptions also cause an error code to be pushed on the stack. 
An exception handler can use the error code to help diagnose the exception.

-   An interrupt procedure also differs from a normal procedure in the method of leaving the procedure. 
The IRET instruction is used to exit from an interrupt procedure. 
IRET is similar to RET except that IRET increments EIP by an extra four bytes (because of the flags on the stack) 
and moves the saved flags into the EFLAGS register. The IOPL field of EFLAGS is changed


so eventually our interrupt_handler looks like: 

                file1.c 

                void initIDT()
                {
                    setup_idt(xxx, (u32int)interrupt_handler);
                }

                extern void interrupt_handler();




                file2.asm 
  
                global interrupt_handler
                
                interrupt_handler: 

                    pusha 

                    call myC_level_interrupt_handler

                    popa 
                    iret 


                file3.c 

                void myC_level_interrupt_handler()
                {
                    ... do my shit ...
                }


again, this is the general structure. there are more details that we will discuss late ron. 


by the way, the structure we mentioned above is also in sync with the explanation given the osdev. 
(https://wiki.osdev.org/Interrupt_Service_Routines)
two stage assembly wrapping 


                isr.asm 

                /* filename: isr_wrapper.s */
                .globl   isr_wrapper
                .align   4
                 
                isr_wrapper:
                    pushad
                    cld /* C code following the sysV ABI requires DF to be clear on function entry */
                    call interrupt_handler
                    popad
                    iret


                isr.c 

                /* filename: interrupt_handler.c */
                void interrupt_handler(void)
                {
                    /* do something */
                }
















-   so now lets actaully define the idt and our isr


lets see how we write those. First thing we know is that the first 32 interrupt handlers are mandated by intel, so we have to define those:

if you go to intels manual on Chapter 6.3.1, it defines all the exception that intel mandates.

        0 - Division by zero exception
        1 - Debug exception
        2 - Non maskable interrupt
        3 - Breakpoint exception
        4 - 'Into detected overflow'
        5 - Out of bounds exception
        6 - Invalid opcode exception
        7 - No coprocessor exception
        8 - Double fault (pushes an error code)
        9 - Coprocessor segment overrun
        10 - Bad TSS (pushes an error code)
        11 - Segment not present (pushes an error code)
        12 - Stack fault (pushes an error code)
        13 - General protection fault (pushes an error code)
        14 - Page fault (pushes an error code)
        15 - Unknown interrupt exception
        16 - Coprocessor fault
        17 - Alignment check exception
        18 - Machine check exception
        19-31 - Reserved

quoting wikipedia:
        "Vectors 0–31 are reserved by Intel for processor generated exceptions (general protection fault, page fault, etc.). 
        Though currently only vectors 0–20 are used by the processor, 
        future processors may create incompatibilities for broken software which use these vectors for other purposes."



for more details on each exception, you can look at 
http://www.logix.cz/michal/doc/i386/chp09-08.htm


















so to code these up, we just fill in the idt entries 


                void set_idt_entry(int n, unsigned int handler_address)
                {
                    idt[n].offset_lower = low_16(handler_address);
                    idt[n].segment_selector = KERNEL_CS;
                    idt[n].zeros = 0;
                    idt[n].flags = 0x8E; 
                    idt[n].offset_upper = high_16(handler_address);
                }

            
                idt.c

                void initIDT()
                {
                    idt_descriptor.limit = sizeof(idt_entry) * 256 - 1;
                    idt_descriptor.address = (unsigned int*)(&IDT);

                    set_idt_entry(0, )
                    set_idt_entry(1, )
                    set_idt_entry(2, )
                    set_idt_entry(3, )
                    set_idt_entry(4, )


                }









################################################################
####################### load idt ###############################
################################################################

recall that to load the idt descirptor to the IDTR register, we actually have to use the LIDT instruction:
so we actually have to write some assembly code again.
(https://arjunsreedharan.org/post/99370248137/kernels-201-lets-write-a-kernel-with-keyboard)

so in our initIDT(); function, we have something like:

            
                idt.c

                void initIDT()
                {
                    idt_descriptor.limit = sizeof(idt_entry) * 256 - 1;
                    idt_descriptor.address = (unsigned int*)(&IDT);

                    set_idt_entry(0, )
                    set_idt_entry(1, )
                    set_idt_entry(2, )
                    set_idt_entry(3, )
                    set_idt_entry(4, )


                    load_idt(&idt_descriptor);
                }





######################################################
############# Interrupt Service Routines #############
######################################################

An ISR is called directly by the CPU, and the protocol for calling an ISR differes from calling a C function.
an ISR has to end with the iret opcode, whereas usual C function end with ret or retf. 

(https://wiki.osdev.org/Interrupt_Service_Routines)









    in NASM you can actually define Macros to save you from duplicated code 







                #define SET_ISR_GATE(i) set_idt_gate(i, isr##i())


                extern void isr0();
                extern void isr1();
                extern void isr2();
                extern void isr3();
                extern void isr4();
                extern void isr5();
                extern void isr6();
                extern void isr7();
                extern void isr8();
                extern void isr9();
                extern void isr20();
                extern void isr22();
                extern void isr22();
                extern void isr23();
                extern void isr24();
                extern void isr25();
                extern void isr26();
                extern void isr27();
                extern void isr28();
                extern void isr29();
                extern void isr30();
                extern void isr31();


                void initIDT()
                {
                    idt_descriptor.limit = sizeof(idt_entry_type) * 256 - 1;
                    idt_descriptor.address = (unsigned int)(&IDT);

                    int i=0; 
                    for(i = 0; i<32; i++)
                    {
                        SET_ISR_GATE(i);
                    }

                    idt_load( (unsigned int)&idt_descriptor );
                }




Preprocessor Output 

gcc -E 



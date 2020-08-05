So lets see what the intel manual says
https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.html

        "Like the GDT and LDTs, the IDT is an array
        of 8-byte descriptors (in protected mode). "

        "The base addresses of the IDT should be aligned on an 8-byte boundary to maximize performance of cache line
        fills. The limit value is expressed in bytes and is added to the base address to get the address of the last valid byte.
        A limit value of 0 results in exactly 1 valid byte. Because IDT entries are always eight bytes long, the limit should
        always be one less than an integral multiple of eight (that is, 8N – 1)."

        "The IDT may reside anywhere in the linear address space. As shown in Figure 6-1, the processor locates the IDT
        using the IDTR register. This register holds both a 32-bit base address and 16-bit limit for the IDT."


As you can see, we got a couple of things to do 



#################################################################
########################## idt entry ############################
#################################################################

1st we will want to define our 8-byte descriptor for our IDT table.

so now lets look at intel specs 6.11
https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.html


The IDT may contain 3 kinds of descriptor 
-   Task gates
-   Interrupt gates
-   Trap gates


you can think of it as 
-   Task handler
-   Interrupt handler 
-   Trap handler 



according to this link:
http://littleosbook.github.io/#creating-an-entry-in-the-idt

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
we can just define a struct like below alled idt_entry

this is literally defined just like the picture in the intel spec.

                typedef struct {
                    unsigned short offset_lower;
                    unsigned short segment_selector;
                    unsigned char zeros;
                    unsigned char flags;
                    unsigned short offset_upper;
                } idt_entry;



you can also look at how JamesM tutorials defines it 
http://www.jamesmolloy.co.uk/tutorial_html/4.-The%20GDT%20and%20IDT.html

or the osdev defines it 
https://wiki.osdev.org/Interrupts_tutorial

its pretty much the same thing 




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



Obviously we want 256 of those, so we have 

                idt.c

                typedef struct {
                    unsigned short offset_lower;
                    unsigned short segment_selector;
                    unsigned char zeros;
                    unsigned char flags;
                    unsigned short offset_upper;
                } idt_entry;

                struct idt_entry IDT[256];


in case you are confused with our typedef works, recall that in C, you would usually do something like 
                
                typedef unsigned char BYTE;

or 
                
                typedef unsigned char byte;

you are literally defining a class, or an alias. I like to think of it as a #define


##########################################################################
############################### idt_descriptor ###########################
##########################################################################

then according to  

        "The base addresses of the IDT should be aligned on an 8-byte boundary to maximize performance of cache line
        fills. The limit value is expressed in bytes and is added to the base address to get the address of the last valid byte.
        A limit value of 0 results in exactly 1 valid byte. Because IDT entries are always eight bytes long, the limit should
        always be one less than an integral multiple of eight (that is, 8N – 1)."


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

                idt.c
                
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

now we have our idt table entry our idt descriptor defined.



#################################################################
#################### IDT initialization #########################
#################################################################

now that we have our idt entry struct defined and our idt descriptor struct defined
we will want to start filling in data inside. 

here we do 3 things 

1.  fill in data inside the idt_descriptor
2.  fill in data inside the idt table 
3.  load the IDTR register


so lets us write a function that acutally fills up an entry in our idt 
for this, you can refer to https://wiki.osdev.org/Interrupts_tutorial



                int kernel_main()
                {
                   test_print();

    ---------->    initIDT();

                   for(;;);
                }

            
                idt.c

                void initIDT()
                {
                    idt_descriptor.limit = sizeof(idt_entry_type) * 256 - 1;
                    idt_descriptor.address = (unsigned int)(&IDT);

                    set_idt_gate(0, (unsigned int)&isr0);
                    set_idt_gate(1, (unsigned int)&isr1);
                    set_idt_gate(2, (unsigned int)&isr2);

                    ...
                    ...

                    set_idt_gate(31, (unsigned int)&isr31);

                    idt_load( (unsigned int)&idt_descriptor );
                }


lets take a look at the set_idt_gate();


                void set_idt_gate(int n, unsigned int handler_address)
                {
                    IDT[n].offset_lower = low_16(handler_address);
                    IDT[n].segment_selector = KERNEL_CODE_SEGMENT;
                    IDT[n].zeros = 0;
                    IDT[n].flags = 0x8E; 
                    IDT[n].offset_upper = high_16(handler_address);
                }


##########################################################################
######################## The Exception Handlers ##########################
##########################################################################

so now we have finished defining the idt_entry class, we now need to concern ourselves with the actual 
interrupt handlers. Interrupt handlers are also called interrupt service routines. 

so in the code snippet above, we had 

                idt.c

                void initIDT()
                {
                    idt_descriptor.limit = sizeof(idt_entry_type) * 256 - 1;
                    idt_descriptor.address = (unsigned int)(&IDT);

    ------------>   set_idt_gate(0, (unsigned int)&isr0);
                    set_idt_gate(1, (unsigned int)&isr1);
                    set_idt_gate(2, (unsigned int)&isr2);

                    ...
                    ...

                    set_idt_gate(31, (unsigned int)&isr31);

                    idt_load( (unsigned int)&idt_descriptor );
                }

you may wonder where did the isr0(); come from.

So the isr0 ~ isr31 are our actual interrupt handlers. The thing with the funciton isr0() is that we can 
either define this in the C file or the in the assembly file.
However, since we have to write some assembly code, we will write this isr0 in the assembly file.
So what we will do is that we will declare it as extern, that way our c file can access it funct,
but we define the isr0 in an assembly file. In short, we are doing this cuz its more convenient this way 
to include the assembly code.


                idt.c 

                extern void isr0();
                extern void isr1();
                extern void isr2();
                ...
                ...
                extern void isr31();

                void initIDT()
                {
                    set_idt_gate(0, (unsigned int)&isr0);
                    ...
                    ...
                }


so then we define these functions in our assembly file 
So roughly, it will look like this 


                idt.asm 
  
                global isr0

                isr0:                     
                    ....................................
                    ....................................
                    ....................................
                    .... my interrupt handler logic ....
                    ....................................
                    ....................................
                    ....................................





so now we need to understand what to actually do in our interrupt handler logic code 


#######################################################################
##################### Handling Exception ##############################
#######################################################################

So lets look at what the Linux Kernel says 

I do want to make the distinction that there is Exceptions and Interrupts. 
for the first 32 interrupts, we want to handle them as exceptions. 
for the IRQ (which we will discuss in the next chapter), we will handle them as interrupts. 


there is a section called "Hardware Handling of Interrupts and Exceptions" at Chapter 4
https://doc.lagout.org/operating%20system%20/linux/Understanding%20Linux%20Kernel.pdf

first, on page 148, it says 

        Exception handlers have a standard structure consisting of three steps:
        1. Save the contents of most registers in the Kernel Mode stack (this part is coded in assembly language).
        2. Handle the exception by means of a high-level C function.
        3. Exit from the handler by means of the ret_from_exception( ) function.


On Page 149 ~ 150 it goes into more detail 


        1. Saves the registers that might be used by the high-level C function on the stack.

        2. Issues a cld instruction to clear the direction flag DF of eflags, thus making sure that autoincreases on the edi and esi 
        registers will be used with string instructions.*

        3. Copies the hardware error code saved in the stack at location esp+36 in edx.
        Stores the value –1 in the same stack location. As we’ll see in the section “Reexecution of System Calls” in Chapter 11, this value is used to separate 0x80 exceptions from other exceptions.
        
        4. Loads edi with the address of the high-level do_handler_name( ) C function
        saved in the stack at location esp+32; writes the contents of es in that stack
        location.
        
        5. Loads in the eax register the current top location of the Kernel Mode stack. This
        address identifies the memory cell containing the last register value saved in
        step 1.
        
        6. Loads the user data Segment Selector into the ds and es registers.
        
        7. Invokes the high-level C function whose address is now stored in edi.

then finally at the end, on page 143 it says 

        After the interrupt or exception is processed, the corresponding handler must relinquish control to the 
        interrupted process by issuing the iret instruction.




we will just do a "pusha" command to push all the general registers 

                idt.asm 

                global isr0

                isr0:                     
    ----------->    pusha
                    ...
                    ...

                    popa
                    iret


then for number 2, we add the cld command 
then we go on to call the interrupt_handler 


                idt.asm 

                global isr0

                isr0:                     
                    pusha
                    cld
                    call interrupt_handler_0 
                    popa
                    iret


step 3, 4, 5, 6, 7. we will ignore for now (this will be addressed later);

obvioulsy we want to write our interrupt_handler_0 in C, so we do the following 
we declare interrupt_handler_0 to be extern, and define it in a C file. 

                idt.asm 

                global isr0

                extern interrupt_handler_0

                isr0:                     
                    pusha
                    cld
                    call interrupt_handler_0 
                    popa
                    iret



                isr.c 

                void interrupt_handler_0(void)
                {
                    /* do something */
                }




################################################################################
####################### Error Code and registers ############################### 
################################################################################

But as you can see, that was our brute force implementation, but one thing we forgot is to have the c level function 
to have the error code and register information

Recall that 

So there are times where we want to access the error code in the handler. 

Recall mentioned above, we have: 

there is a section called "Hardware Handling of Interrupts and Exceptions" at Chapter 4
https://doc.lagout.org/operating%20system%20/linux/Understanding%20Linux%20Kernel.pdf

        8. If the exception carries a hardware error code, it saves it on the stack.
        
so there are times in our interrupt handler, we need to get that error code information. So we want to get that.
So when we call isr_handler(), we want the c function to have access to the error code 

so we are gonna want something like 


                isr.c 

                void interrupt_handler_14(error_code + register info)
                {
                    /* do something */
                }

This is where we will diverge from "Understanding the linux kernel". On Page 150, it says 

        "The invoked function receives its arguments from the eax and edx registers rather
        than from the stack."

Bran_s Kernel Development gets it from the stack. For this tutorial, we will do what Bran_s Kernel Development did,
and get it from the stack.

so to draw a picture to see what our stack looks like when 


                 _____________________
                |                     |
                |                     |
                |_____________________|
                |                     |
                |                     |
                |_____________________|
                |                     |
                |                     |
                |_____________________|
                |   call c level      |
                |      handler        | 
                |_____________________|
                |                     |
                |       edi           |
                |       esi           |
                |       ebp           |
                |     original esp    |
                |       ebx           |
                |       edx           |                
                |       ecx           | <----- done by us with the pusha command (order matters)
                |       eax           |
                |_____________________|                
                |    interrupt_num    |
                |    error code       | <------ done by hardware
                |_____________________|
                |       eip           |       
                |       cs            |  
                |      eflags         | <------ done by hardware (order matters)
                |_____________________|
                |                     |
                |                     |
                |_____________________|


so for the handler to get all the register info and error code, we can just get it from the stack.

Recall to do that in assembly language, you just push your function arguments on to the stack 

                
                push arg3 
                push arg2 
                push arg1
                call function()
                ...
                ...


so we can do something similar to access all the error_code and register info

                interrupt.asm

                isr14:   
                    push byte 14                  
                    pusha
                    cld
                    mov eax, esp    <-------------
                    push eax        <-------------                    
                    call interrupt_handler_14       
                    add esp, 4  
                    popa
                    add esp, 8
                    iret

                isr.c

                /* This defines what the stack looks like after an ISR was running */
                struct register_info
                {
                    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pushed by 'pusha' */
                    unsigned int interrupt_num, err_code;    /* our 'push byte #' and ecodes do this */
                    unsigned int eip, cs, eflags, useresp, ss;   /* pushed by the processor automatically */ 
                };
                

                void interrupt_handler_14(register_info *info)
                {
                    kprint("isr_handler 14\n");
                    page_fault_handler();
                }


you can see that we just define a struct that matches out stack, which is the register_info struct.
and in the interrupt_handler_14, we get the pointer to it by using the two instructions 

                mov eax, esp    <-------------
                push eax        <-------------


one thing is that when you call "push byte 14", the push instruction is by default always 32 bits.
so in our register_info struct, we have the interrupt_num as an unsigned int.


according to the spec 
https://c9x.me/x86/html/file_module_x86_id_270.html
The pusha command 

                Push EAX, ECX, EDX, EBX, original ESP, EBP, ESI, and EDI.

so we just order them in reverse in the register_info struct. 


-   linux_s approach

If you look at "Understanding the Linux Kernel" page 162, Section "Saving the registers for the interrupt handler"

there is a code snippet

                common_interrupt:
                    SAVE_ALL
                    movl %esp,%eax          <---------------
                    call do_IRQ
                    jmp ret_from_intr


where SAVE_ALL is

                cld
                push %es
                push %ds
                pushl %eax
                pushl %ebp
                pushl %edi
                pushl %esi
                pushl %edx
                pushl %ecx
                pushl %ebx
                movl $_ _USER_DS,%edx
                movl %edx,%ds
                movl %edx,%es


the book also says 

        SAVE_ALL saves all the CPU registers that may be used by the interrupt handler on the stack, except for eflags, cs, eip, ss and esp, 
        which ar already saved automatically by the control unit.

        After saving the registers, the address of the current top stack location is saved in the
        eax register;

                _ _attribute_ _((regparm(3))) unsigned int do_IRQ(struct pt_regs *regs)

        The regparm keyword instructs the function to go to the eax register to find the value
        of the regs argument; as seen above, eax points to the stack location containing the
        last register value pushed on by SAVE_ALL.



You can actually look at how the |pt_regs| struct is defined from google since Linux src code is all open to us
https://github.com/torvalds/linux/blob/v4.8/arch/x86/include/uapi/asm/ptrace.h

                struct pt_regs {
                    long ebx;
                    long ecx;
                    long edx;
                    long esi;
                    long edi;
                    long ebp;
                    long eax;
                    int  xds;
                    int  xes;
                    int  xfs;
                    int  xgs;
                    long orig_eax;
                    long eip;
                    int  xcs;
                    long eflags;
                    long esp;
                    int  xss;
                };

so its very similar to Bran_s Kernel implementation


################################################################################
################################ All 32 ISRs ############################### 
################################################################################
And recall, there are exceptions that pushes an error code and there are those that dont. 

OS dev has a table that describes all of it
https://wiki.osdev.org/Exceptions

So for the ones we dont have error code pushed by the hardware, we have:

                idt.asm 

                global isr0
                global isr1
                global isr2
                ...
                ...

                extern interrupt_handler_0
                extern interrupt_handler_1
                ...
                ...
                extern interrupt_handler_14


                isr0: 
                    push byte 0    <---- fake error code
                    push byte 0    <---- interrupt number               
                    pusha
                    cld
                    call interrupt_handler_0 
                    add esp, 4                      
                    popa
                    add esp, 8
                    iret

                ...
                ...

                isr14:   
                    push byte 14                  
                    pusha
                    cld
                    mov eax, esp    
                    push eax                            
                    call interrupt_handler_14       
                    add esp, 4  
                    popa
                    add esp, 8
                    iret

                ...
                ...



so we can write a general function to combine some of these repetitive code
also the thing is that ones like isr0 wont have a error code pushed onto the stack. 
So assuming every handler is gonna want the error code and the register situation,
meaning all of isr0 ~ isr31 would want register_info *info, then we can just make some of the logic common.

The trick is to push error code of 0 for the ones that the hardware wont push 
so our code looks like: 

                idt.asm 

                global isr0
                global isr1
                global isr2
                ...
                ...

                extern interrupt_handler_0
                extern interrupt_handler_1
                ...
                ...
                extern interrupt_handler_14


                isr0:                     
                    push dword 0
                    jmp common_interrupt_handler
                ...
                ...

                isr14:                     
                    jmp common_interrupt_handler

                common_interrupt_handler:

                    pusha
                    cld
                    mov eax, esp    
                    push eax                            
                    call interrupt_handler_14       
                    popa
                    iret
                ...
                ...




##################################################################
################################# Testing ######################## 
##################################################################

To test my IDT, we issue a software interrupt 

                test_idt:
                    INT 0x2
                    ret;




##################################################################
################ Loading the IDT Register ######################## 
##################################################################

lets look at assembly code of the load_idt function

                idt.asm

                load_idt:
                    mov eax, [esp + 4]
                    lidt [eax]
                    ret


in a lot of tutorial, such as this one 
https://wiki.osdev.org/Interrupts_tutorial
which as the "sti" command. 

                load_idt:
                    mov eax, [esp + 4]
                    lidt [eax]
                    sti
                    ret

for some reason, adding the "sti" command, makes my go into a loop. Meaning it goes back into 
line 1 of kernel.c. and then it goes back to kernel_main(); 

but I found a different link that doesnt have the "sti" command 
https://littleosbook.github.io/#loading-the-idt


the specs for "sti" is, which says: 
https://c9x.me/x86/html/file_module_x86_id_304.html




##################################################################
################# __attribute__((packed)) ######################## 
##################################################################


so onething I forgot to mention is that when we defined the structs for idt_entry 
and idt_descriptor, we need to add the __attribute__((packed)), otherwise the c compiler will 
add padding to your structs for data alignment. 

                struct idt_entry_struct {
                    unsigned short offset_lower;
                    unsigned short segment_selector;
                    unsigned char zeros;
                    unsigned char flags;
                    unsigned short offset_upper;
                } __attribute__((packed));


                struct idt_descriptor_struct{
                    unsigned short limit; 
                    unsigned int address;
                } __attribute__((packed));


there are actually different ways to tell the compiler to tightly pack your data 
you can see the link here. 
https://www.geeksforgeeks.org/how-to-avoid-structure-padding-in-c/


you can do the __attribute__((packed)) way or the #pragma pack(1) way. This is what Casey did 
on handmade hero. 

so as a matter of fact, in my first run without the pack, when I call INT 0x2 to test our my IDT, isr2(); wasnt getting called. 

and when I examined the memory of idt_descriptor with gdb, this is what I see 

                (gdb) print idt_descriptor
                $1 = {limit = 2047, address = 12992}

so if you just call print idt_descriptor, it looks normal, where we have the limit and its address properly assigned.

but when we actually examine its memory

                (gdb) x/10b 0x3ac0
                0x3ac0 <idt_descriptor>:        0xff    0x07    0x00    0x00    0xc0    0x32    0x00    0x00    0x00    0x00

recall the structure of the idt_descriptor is a 

                unsigned short limit; 
                unsigned int address;

you can see there is 2 byte padding after the "unsigned short limit"

and after I added the __attribute__((packed)), INT 0x2 now calls into isr2(); which is beautiful.

so the lesson here is anything that intel expects to be a tightly compact data structure, we need 
to tightly pack it. 
















#####################################################################################
######################## dont go below here #########################################
#####################################################################################
#####################################################################################
#####################################################################################
#####################################################################################
#####################################################################################
#####################################################################################








this link at section 6.4 has a section called "6.4 Creating a Generic Interrupt Handler"
https://littleosbook.github.io/#handling-an-interrupt

recall there are handlers that has error code and dont has error codes 



                %macro no_error_code_interrupt_handler %1
                global interrupt_handler_%1
                interrupt_handler_%1:
                    push    dword 0                     ; push 0 as error code
                    push    dword %1                    ; push the interrupt number
                    jmp     common_interrupt_handler    ; jump to the common handler
                %endmacro

                %macro error_code_interrupt_handler %1
                global interrupt_handler_%1
                interrupt_handler_%1:
                    push    dword %1                    ; push the interrupt number
                    jmp     common_interrupt_handler    ; jump to the common handler
                %endmacro


                common_interrupt_handler:               ; the common parts of the generic interrupt handler
                    ; save the registers
                    pusha

                    ; call the C function
                    call    interrupt_handler

                    ; restore the registers
                    popa

                    ; restore the esp
                    add     esp, 8

                    ; return to the code that got interrupted
                    iret


                idt.asm 

                global isr0
                global isr1
                global isr2
                ...
                ...

                extern interrupt_handler_0
                extern interrupt_handler_1
                extern interrupt_handler_2
                ...
                ...

                isr0:                     
                    no_error_code_interrupt_handler

                isr1:                     
                    no_error_code_interrupt_handler


                ...
                ...



                no_error_code_interrupt_handler 0       ; create handler for interrupt 0
                no_error_code_interrupt_handler 1       ; create handler for interrupt 1
                .
                .
                .
                error_code_handler              7       ; create handler for interrupt 7
                .
                .
                .




















Error Code 
http://www.logix.cz/michal/doc/i386/chp09-10.htm




so according to this link: 
https://linux-kernel-labs.github.io/refs/heads/master/lectures/interrupts.html

        "So when an interrupt occurs, it pushes the EFLAGS register before saving the address of the interrupted 
        instruction. Certian types of exception also cause an error code to be pushed on the stack help debug the exception"

so this automatically happens. we dont have to do anything to have these 3 assembly instruction happen. 


-   Handling an interrupt request
After an interrupt request has been generated the processor runs a 
sequence of events that eventually ends up with running the kernel interrupt handler:

-   CPU checks the current privilege level

-   if need to change privilege level

    change stack with the one associated with new privilege
    save old stack information on the new stack

-   save EFLAGS, CS, EIP on stack

-   save error code on stack in case of an abort

-   execute the kernel interrupt handler



-   Returning from an interrupt handler
Most architectures offers special instructions to clean-up the stack and resume the execution after the interrupt handler has been executed. On x86 IRET is used to return from an interrupt handler. IRET is similar with RET except that IRET increments ESP by extra four bytes (because of the flags on stack) and moves the saved flags into EFLAGS register.

To resume the execution after an interrupt the following sequence is used (x86):

-   pop the eror code (in case of an abort)

-   call IRET
    pops values from the stack and restore the following register: CS, EIP, EFLAGS

    if privilege level changed returns to the old stack and old privilege level




so in terms of code, so this is what we have to do in terms of cude.





                idt.asm 
  
                global isr0

                isr0:                                         
                    cli 
                    push byte 0
                    push byte 0
                    pusha           ; Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax
                    mov ax, ds      ; Lower 16-bits of eax = ds.
                    push eax        ; save the data segment descriptor
                    mov ax, 0x10    ; kernel data segment descriptor
                    mov ds, ax
                    mov es, ax
                    mov fs, ax
                    mov gs, ax
                    
                    ; 2. Call C handler
                    call isr_handler
                    
                    ; 3. Restore state
                    pop eax 
                    mov ds, ax
                    mov es, ax
                    mov fs, ax
                    mov gs, ax
                    popa
                    add esp, 8      ; Cleans up the pushed error code and pushed ISR number
                    sti
                    iret            ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP











referring to https://doc.lagout.org/operating%20system%20/linux/Understanding%20Linux%20Kernel.pdf
Chapter 4, Preliminary Initialization of the IDT
it mentions 

        The ignore_int( ) interrupt handler, which is in assembly language, may be viewed
        as a null handler that executes the following actions:
        1. Saves the content of some registers in the stack.
        2. Invokes the printk( ) function to print an “Unknown interrupt” system message.
        3. Restores the register contents from the stack.
        4. Executes an iret instruction to restart the interrupted program.

so for now we can just do this.









so if you look at the Linux Kernel.pdf 
https://doc.lagout.org/operating%20system%20/linux/Understanding%20Linux%20Kernel.pdf

In Chapter 4, there is a section called I/O Interrupt Handling 
it says 
                
        "Regardless of the kind of circuit that caused the interrupt, all I/O interrupt handlers
        perform the same four basic actions:
        1. Save the IRQ value and the register’s contents on the Kernel Mode stack.
        2. Send an acknowledgment to the PIC that is servicing the IRQ line, thus allowing it to issue further interrupts.
        3. Execute the interrupt service routines (ISRs) associated with all the devices that share the IRQ.
        4. Terminate by jumping to the ret_from_intr( ) address"



https://wiki.osdev.org/Interrupts_tutorial


we see that we would want 

                idt.asm 
  
                global isr0

                isr0:                     
                    pusha
                    call irq0_handler
                    popa
                    iret

                then for the interrupt_handler function, we can define that in the c file 


pusha: eax, ebx, ecx, edx, esp, ebp, esi and edi 
cld: clears the Driection flag in the EFLAGS register. When the DF flag is set to 0, string 
operations increment the index registers (ESI and/or EDI)






the chapter also says 

        "
        Saving the registers for the interrupt handler
        
        When a CPU receives an interrupt, it starts executing the code at the address found
        in the corresponding gate of the IDT (see the earlier section “Hardware Handling of
        Interrupts and Exceptions”)."


1.  Saving registers is the first task of the interrupt handler


        idt.asm 

        global isr0

        isr0:    
             pushl $n-256
             jmp common_interrupt

        common_interrupt:
            SAVE_ALL
            movl %esp,%eax
            call do_IRQ
            jmp ret_from_intr

the SAVE_ALL macro has 

            cld
            push %es
            push %ds
            pushl %eax
            pushl %ebp
            pushl %edi
            pushl %esi

            pushl %edx
            pushl %ecx
            pushl %ebx
            movl $_ _USER_DS,%edx
            movl %edx,%ds
            movl %edx,%es



            popl %ebx
            popl %ecx
            popl %edx
            popl %esi
            popl %edi
            popl %ebp
            popl %eax
            popl %ds
            popl %es
            addl $4, %esp
            iret





Then in our assembly file, we write out our routine
So what we have to do inside isr0, we can once again refer to the intel manual, 6.12.1 Exception- or Interrupt-Handler procedures
        
        "When the processor performs a call to the exception- or interrupt-handler procedure:
        • If the handler procedure is going to be executed at a numerically lower privilege level, a stack switch occurs.
        When the stack switch occurs:
        a. The segment selector and stack pointer for the stack to be used by the handler are obtained from the TSS
        for the currently executing task. On this new stack, the processor pushes the stack segment selector and
        stack pointer of the interrupted procedure.
        b. The processor then saves the current state of the EFLAGS, CS, and EIP registers on the new stack (see
        Figures 6-4).
        c. If an exception causes an error code to be saved, it is pushed on the new stack after the EIP value.
        • If the handler procedure is going to be executed at the same privilege level as the interrupted procedure:
        a. The processor saves the current state of the EFLAGS, CS, and EIP registers on the current stack (see
        Figures 6-4).
        b. If an exception causes an error code to be saved, it is pushed on the current stack after the EIP value."


then
        
        "To return from an exception- or interrupt-handler procedure, the handler must use the IRET (or IRETD) instruction.
        The IRET instruction is similar to the RET instruction except that it restores the saved flags into the EFLAGS
        register. The IOPL field of the EFLAGS register is restored only if the CPL is 0. The IF flag is changed only if the CPL
        is less than or equal to the IOPL."


so our code looks like: 


                idt.asm 
  
                global isr0

                isr0: 
                    pusha               ; Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax

                    mov ax, ds          ; Lower 16-bits of eax = ds.
                    push eax            ; save the data segment descriptor

                    mov ax, 0x10        ; load the kernel data segment descriptor
                    mov ds, ax
                    mov es, ax
                    mov fs, ax
                    mov gs, ax

                    .... my interrupt handler logic ....

                    pop eax              ; reload the original data segment descriptor
                    mov ds, ax
                    mov es, ax
                    mov fs, ax
                    mov gs, ax

                    popa                 ; Pops edi,esi,ebp...
                    add esp, 8           ; Cleans up the pushed error code and pushed ISR number
                    sti
                    ret                 ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP



the .... my interrupt_handler logic .... can be in c, so you can have something like: 

                idt.asm 
  
                global isr0

                isr0: 

                    cli                 ; Disable interrupts
                    push byte 0         ; Push a dummy error code (if ISR0 doesn't push it's own error code)
                    push byte 0         ; Push the interrupt number (0)
                    pusha               ; Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax

                    mov ax, ds          ; Lower 16-bits of eax = ds.
                    push eax            ; save the data segment descriptor

                    mov ax, 0x10        ; load the kernel data segment descriptor
                    mov ds, ax
                    mov es, ax
                    mov fs, ax
                    mov gs, ax

                    call myC_level_interrupt_handler

                    pop eax              ; reload the original data segment descriptor
                    mov ds, ax
                    mov es, ax
                    mov fs, ax
                    mov gs, ax

                    popa                 ; Pops edi,esi,ebp...
                    add esp, 8           ; Cleans up the pushed error code and pushed ISR number
                    sti
                    ret                 ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP






so there are a two problems with this piece of code 

1.  iret vs ret 
so if you look at this link:  
http://www.logix.cz/michal/doc/i386/chp09-06.htm

it says
        
        "The 80386 invokes an interrupt or exception handling procedure 
        in much the same manner as it CALLs a procedure; the differences are explained in the following sections.

        An interrupt procedure also differs from a normal procedure in the method of leaving the procedure. 
        The IRET instruction is used to exit from an interrupt procedure. 
        IRET is similar to RET except that IRET increments EIP by an extra four bytes (because of the flags on the stack) 
        and moves the saved flags into the EFLAGS register. The IOPL field of EFLAGS is changed"



Multiple links mentions the same thing 
https://wiki.osdev.org/Interrupt_Service_Routines

        "An ISR is called directly by the CPU, and the protocol for calling an ISR differs from calling e.g. a C function. 
        Most importantly, an ISR has to end with the iret opcode, whereas usual C functions end with ret or retf."



http://www.jamesmolloy.co.uk/tutorial_html/4.-The%20GDT%20and%20IDT.html
        
        "The IRET instruction is specifically designed to return from an interrupt. 
        It pops these values off the stack and returns the processor to the state it was in originally."


so in our routine we have to change ret to iret 

                idt.asm 
                
                global isr0

                isr0: 

                    pusha 

                    call myC_level_interrupt_handler

                    popa 
                    iret 


2.  2nd problem we have is mention in this link as well 
http://www.logix.cz/michal/doc/i386/chp09-06.htm

        "also note that certain types of exceptions also cause an error code to be pushed on the stack. 
        An exception handler can use the error code to help diagnose the exception."

so for all of our interrupt routine, we can have a common routine like below:












so eventually our interrupt_handler looks like: 

                idt.c 

                extern void isr0();
                extern void isr1();
                extern void isr2();
                ...
                ...
                extern void isr31();


                void initIDT()
                {
                    set_idt_gate(0, (unsigned int)&isr0);
                    ...
                    ...
                }

    --------------------------------------------

                idt.asm 
  
                global isr0
                global isr1
                ...
                ...
                global isr31

                isr0: 

                    pusha 

                    call myC_level_interrupt_handler

                    popa 
                    iret 
                
                ...
                ...

                isr31: 

                    pusha 

                    call myC_level_interrupt_handler

                    popa 
                    iret 

    --------------------------------------------

                file3.c 

                void myC_level_interrupt_handler()
                {
                    ... do my shit ...
                }


again, this is the general structure. there are more details that we will discuss late ron. 


by the way, the structure we mentioned above is also in sync with the explanation given the osdev. 
(https://wiki.osdev.org/Interrupt_Service_Routines)
The osdev link has this "two stage assembly wrapping" section which is exactly what we did.


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




######################################################################
######################### idt_load(); ################################                
######################################################################

so we have defined our interrupt handler routine, now its time to load the data into the LDTR register.
again its easier to do this in assembly, so we define an extern idt_load(); function 

            
                idt.c

    -------->   extern void idt_load(unsigned int);

                void initIDT()
                {
                    idt_descriptor.limit = sizeof(idt_entry_type) * 256 - 1;
                    idt_descriptor.address = (unsigned int)(&IDT);

                    set_idt_gate(0, (unsigned int)&isr0);
                    set_idt_gate(1, (unsigned int)&isr1);
                    set_idt_gate(2, (unsigned int)&isr2);

                    ...
                    ...

                    set_idt_gate(31, (unsigned int)&isr31);

    ----------->    idt_load( (unsigned int)&idt_descriptor );
                }




and then the actual definition lives in an assembly file 

                idt.asm 

                global idt_load

                idt_load:
                    mov edx, [esp + 4]
                    lidt [edx]
                    sti
                    ret

if you dont want to go through the assembly route, you can just do it in line assembly in c 

                void idt_load(unsigned int descriptor_address) {
                    __asm__ __volatile__("lidtl (%0)" : : "r" (descriptor_address));
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




We set the isr handlers to be external, so we can access the addresses of our ASM ISR handlers.




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














we will just do a "pusha" command to push all the general registers 

                idt.asm 

                global isr0

                isr0:                     
    ----------->    pusha
                    ...
                    ...

                    popa
                    iret








The invoked function receives its arguments from the eax and edx registers rather
than from the stack. 


So the code snippet provided on page 149 is:

        handler_name:
            pushl $0 /* only for some exceptions */
            pushl $do_handler_name
            jmp error_code









        After executing an instruction, the cs and eip pair of registers contain the logical
        address of the next instruction to be executed. Before dealing with that instruction, the
        control unit checks whether an interrupt or an exception occurred while the control unit
        executed the previous instruction. If one occurred, the control unit does the following:


        5. if there is a privilege level change, we save the previous values of ss and esp.

        6. If a fault has occurred, it loads cs and eip with the logical address of the instruction that caused the exception so that it can be executed again.
        
        7. Saves the contents of eflags, cs, and eip in the stack.

        8. If the exception carries a hardware error code, it saves it on the stack.
        
        9. Loads cs and eip, respectively, with the Segment Selector and the Offset fields of
        the Gate Descriptor stored in the ith entry of the IDT. These values define the
        logical address of the first instruction of the interrupt or exception handler.


Note that these are executed by the Processor control unit, so these are all executed by the CPU itself. You 
dont have to do any of this. Our isr0 code is going straight to "the first instruction of the interrupt or exception handler."


        After the interrupt or exception is processed, the corresponding handler must relinquish control to the 
        interrupted process by issuing the iret instruction




so we add our iret instruction 

IRET: pops values from the stack and restore the following register: CS, EIP, EFLAGS

                idt.asm 

                global isr0

                isr0:                     
                    ....................................
                    ....................................
                    ....................................
                    .... my interrupt handler logic ....
                    ....................................
                    ....................................
                    ....................................

    ----------->    iret



#######################################################################
##################### Handling Interrupts #############################
#######################################################################



Page 149 ~ 150 goes into detail of how we do this in detail


                1. Saves the registers that might be used by the high-level C function on the stack.

                2. Issues a cld instruction to clear the direction flag DF of eflags, thus making sure
                that autoincreases on the edi and esi registers will be used with string
                instructions.*
                
                3. Copies the hardware error code saved in the stack at location esp+36 in edx.
                Stores the value –1 in the same stack location. 
                As we’ll see in the section “Reexecution of System Calls” in Chapter 11, this value is used to separate 0x80 exceptions from other exceptions.
                
                4. Loads edi with the address of the high-level do_handler_name( ) C function
                saved in the stack at location esp+32; writes the contents of es in that stack
                location.
                
                5. Loads in the eax register the current top location of the Kernel Mode stack. This
                address identifies the memory cell containing the last register value saved in
                step 1.
                
                6. Loads the user data Segment Selector into the ds and es registers.

                7. Invokes the high-level C function whose address is now stored in edi.




So I am going to piece together the code from the "Understanding the Linux Kernel"

so for note number 1, "Saves the registers that might be used by the high-level C function on the stack."
We have                

                handler_name:
                    pushl $0 /* only for some exceptions */
                    pushl $do_handler_name
                    jmp error_code

then on page 162, section "Saving the registers for the interrupt handler", it goes into further detail 
on how to save the registers for the interrupt handler. 

                common_interrupt:
                    SAVE_ALL
                    movl %esp,%eax
                    call do_IRQ
                    jmp ret_from_intr


and SAVE_ALL is 

                cld
                    push %es
                    push %ds
                    pushl %eax
                    pushl %ebp
                    pushl %edi
                    pushl %esi

                    pushl %edx
                    pushl %ecx
                    pushl %ebx
                    movl $_ _USER_DS,%edx
                    movl %edx,%ds
                    movl %edx,%es





There seems to be some disagreemnt on the order of 1 and 2 (not sure if it even matters);



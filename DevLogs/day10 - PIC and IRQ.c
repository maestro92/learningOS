
So now we need to setup our PIC in kernel. 

so the thing we need to understand is that we have our IDT and we have our pic that generates all these IRQs. 

IRQs is separate from IDT.

see this graph 
https://hsto.org/webt/xj/yn/dh/xjyndhabujjwz0alrbx831zfzt8.png

So naively, you would think that IRQ 0 should map to ISR 0 in our IDT table. 
and IRQ 1 should map to ISR 1 in our IDT table 

                IDT   handles   PIC                 

                ISR 0 --------> IRQ 0 (system timer)
                ISR 1 --------> IRQ 1 (keyboard)
                ISR 2 --------> IRQ 2
                ISR 3 --------> IRQ 3
                ISR 4 --------> IRQ 4

                ...
                ...

                ISR 5 --------> IRQ 5
                ISR 6 --------> IRQ 6



but on the other hand, you also remember intel saying 

        "Vector numbers in the range 0 through 31 are reserved by the
        Intel 64 and IA-32 architectures for architecture-defined exceptions and interrupts.

        Vector numbers in the range 32 to 255 are designated as user-defined interrupts and are not reserved by the Intel
        64 and IA-32 architecture. "

        https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.html


so what the faq?

turns out that you dont have to map IRQ 0 to ISR 0. You want to map IRQ handlers in the user-defined interrupts in the ragen of 32 to 255.
so somethingl ike 



                IDT   handles   PIC                 

                ISR 0 --------> divide by zero
                ISR 1 --------> 
                ISR 2 --------> IRQ 2
                ISR 3 --------> IRQ 3
                ISR 4 --------> IRQ 4

                ...
                ...

                ISR 32 --------> IRQ 0 (system timer)
                ISR 33 --------> IRQ 1 (keyboard)
                ISR 34 --------> IRQ 2
                ISR 35 --------> IRQ 3
                ISR 36 --------> IRQ 4

                ...
                ...


so quoting the article from OS dev:

so according to this link: 
https://wiki.osdev.org/8259_PIC

it says     

        In protected mode, the IRQs 0 to 7 conflict with the CPU exception which are reserved by Intel up until 0x1F.  (0x1F is 31);
        (It was an IBM design mistake.) Consequently it is difficult to tell the difference between an IRQ or an software error. 
        It is thus recommended to change the PICs offsets (also known as remapping the PIC) so that IRQs use non-reserved vectors. 
        A common choice is to move them to the beginning of the available range (IRQs 0...0xF -> INT 0x20...0x2F). 
        For that, we need to set the master PIC_s offset to 0x20 and the slave_s to 0x28. For code examples, see below.


as mentioned in this link as well, 
https://linux-kernel-labs.github.io/refs/heads/master/lectures/interrupts.html

if you scroll down to the "Architecture specific interrupt handling in Linux" section 

        0 ~ 31 entries are reserved for exceptions 

        32 ~ 127 entries are reserved for device interrupts.


so what we need to do is to map the PIC IRQ 0 ~ 15 to IDT 32 ~ 47


So we will want to remap the PIC IRQs.



################################################################
####################### Initialization #########################
################################################################

So according to this link, it says:
        
        When you enter protected mode (or even before hand, if you_re not using GRUB) 
        the first command you will need to give the two PICs is the initialise command (code 0x11). 
        This command makes the PIC wait for 3 extra "initialisation words" on the data port. These bytes give the PIC:


                Its vector offset. (ICW2)
                Tell it how it is wired to master/slaves. (ICW3)
                Gives additional information about the environment. (ICW4)

        https://wiki.osdev.org/8259_PIC


so again, we are here trying to talk to our PIC, and recall on day07 where we talked about how the CPU talks to hardware devices,
which is through the device registers. Here we do the same thing with PIC. 
The PIC also has registers mapped to a memory address


                Chip - Purpose  I/O port
                Master PIC - Command    0x0020
                Master PIC - Data   0x0021
                Slave PIC - Command 0x00A0
                Slave PIC - Data    0x00A1

                https://wiki.osdev.org/PIC


here you can take a look at the intel spec on 8259
https://pdos.csail.mit.edu/6.828/2010/readings/hardware/8259A.pdf



obviously its a bit too much
but I found some explanation 
https://github.com/arjun024/mkeykernel

so if you look at kernel.c
https://github.com/arjun024/mkeykernel/blob/master/kernel.c

it has pretty good comments on the commands you send to it. So We will just copy what that file does

                void initIDT()
                {
                    idt_descriptor.limit = sizeof(idt_entry_type) * 256 - 1;
                    idt_descriptor.address = (unsigned int)(&IDT);


                    // Remap the PIC
                    /* ICW1 - begin initialization */
                    port_byte_out(0x20, 0x11);
                    port_byte_out(0xA0, 0x11);

                    /* ICW2 - remap offset address of IDT */
                    /*
                    * In x86 protected mode, we have to remap the PICs beyond 0x20 because
                    * Intel have designated the first 32 interrupts as "reserved" for cpu exceptions
                    */
                    port_byte_out(0x21, 0x20);
                    port_byte_out(0xA1, 0x28);

                    /* ICW3 - setup cascading */
                    port_byte_out(0x21, 0x04);
                    port_byte_out(0xA1, 0x02);

                    /* ICW4 - environment info */
                    port_byte_out(0x21, 0x01);
                    port_byte_out(0xA1, 0x01);


                    port_byte_out(0x21, 0x0);
                    port_byte_out(0xA1, 0x0);

                    ...
                    ...
                }







################################################################
####################### x86 IRQs ###############################
################################################################

-   history 

https://en.wikipedia.org/wiki/Interrupt_request_(PC_architecture)

Early PCs using the Intel 8086/8088 processors only had a single PIC, and 
are therefore limited to eight interrupts. This was expanded to two PICs with the introduction of the 286 based PCs.


Typically, on systems using the Intel 8259 PIC, 16 IRQs are used. IRQs 0 to 7 are managed by one Intel 8259 PIC, 
and IRQs 8 to 15 by a second Intel 8259 PIC. The first PIC, the master, is the only one that directly signals the CPU. 
The second PIC, the slave, instead signals to the master on its IRQ 2 line, and the master passes the signal on to the CPU. 
There are therefore only 15 interrupt request lines available for hardware.


so as you can see in the wiki link 
IRQ 0 is for system timer.
IRQ 1 is for keyboard controller.

So lets try to set up the irq for the keyboard controller 

again visually, 
https://hsto.org/webt/xj/yn/dh/xjyndhabujjwz0alrbx831zfzt8.png


        Example (from here):
        IRQ 0 — system timer
        IRQ 1 — keyboard controller
        IRQ 2 — cascade (interrupt from slave controller)
        IRQ 3 — serial port COM2
        IRQ 4 — serial port COM1
        IRQ 5 — parallel port 2 and 3 or sound card
        IRQ 6 — floppy controller
        IRQ 7 — parallel port 1
        IRQ 8 — RTC timer
        IRQ 9 — ACPI
        IRQ 10 — open/SCSI/NIC
        IRQ 11 — open/SCSI/NIC
        IRQ 12 — mouse controller
        IRQ 13 — math co-processor
        IRQ 14 — ATA channel 1
        IRQ 15 — ATA channel 2


In day 11, we will properly handle the keyboard


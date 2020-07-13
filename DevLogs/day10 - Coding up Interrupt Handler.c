

Saving the registers for the interrupt handler









https://wiki.osdev.org/Interrupts
Before the ret, this code is executed, to tell the PIC that its OK to send any new or pending interrupts, 
because the current one is done. The PIC doesnt send any more interrupts until the cpu acknowledges the interrupt:

                mov al,20h
                out 20h,al


In the case of the keyboard input, the interrupt handler asks the keyboard which key was pressed, does something with the information, then acknowledges and return:
                push eax    ;; make sure you dont damage current state
                in al,60h   ;; read information from the keyboard

                mov al,20h
                out 20h,al  ;; acknowledge the interrupt to the PIC
                pop eax     ;; restore state
                iret        ;; return to code executed before.









https://wiki.osdev.org/Interrupts_tutorial

                void irq0_handler(void) {
                    outb(0x20, 0x20); //EOI
                }
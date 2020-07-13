
global isr0
global isr1
global isr2
global isr3
global isr4
global isr5
global isr6
global isr7
global isr8
global isr9
global isr10
global isr11
global isr12
global isr13
global isr14
global isr15
global isr16
global isr17
global isr18
global isr19
global isr20
global isr21
global isr22
global isr23
global isr24
global isr25
global isr26
global isr27
global isr28
global isr29
global isr30
global isr31

global load_idt
global test_idt

extern interrupt_handler_0
extern interrupt_handler_1
extern interrupt_handler_2
extern interrupt_handler_3
extern interrupt_handler_4
extern interrupt_handler_5
extern interrupt_handler_6
extern interrupt_handler_7
extern interrupt_handler_8
extern interrupt_handler_9
extern interrupt_handler_10
extern interrupt_handler_11
extern interrupt_handler_12
extern interrupt_handler_13
extern interrupt_handler_14
extern interrupt_handler_15
extern interrupt_handler_16
extern interrupt_handler_17
extern interrupt_handler_18
extern interrupt_handler_19
extern interrupt_handler_20
extern interrupt_handler_21
extern interrupt_handler_22
extern interrupt_handler_23
extern interrupt_handler_24
extern interrupt_handler_25
extern interrupt_handler_26
extern interrupt_handler_27
extern interrupt_handler_28
extern interrupt_handler_29
extern interrupt_handler_30
extern interrupt_handler_31

isr0:                     
    pusha
    cld
    call interrupt_handler_0 
    popa
    iret

isr1:                    
    pusha
    cld
    call interrupt_handler_1 
    popa
    iret

isr2:                     
    pusha
    cld
    call interrupt_handler_2
    popa
    iret

isr3:                    
    pusha
    cld
    call interrupt_handler_3 
    popa
    iret

isr4:                     
    pusha
    cld
    call interrupt_handler_4 
    popa
    iret

isr5:                    
    pusha
    cld
    call interrupt_handler_5 
    popa
    iret

isr6:                     
    pusha
    cld
    call interrupt_handler_6 
    popa
    iret

isr7:                    
    pusha
    cld
    call interrupt_handler_7 
    popa
    iret

isr8:                     
    pusha
    cld
    call interrupt_handler_8
    popa
    iret

isr9:                    
    pusha
    cld
    call interrupt_handler_9
    popa
    iret

isr10:                     
    pusha
    cld
    call interrupt_handler_10 
    popa
    iret

isr11:                    
    pusha
    cld
    call interrupt_handler_11 
    popa
    iret

isr12:                     
    pusha
    cld
    call interrupt_handler_12
    popa
    iret

isr13:                    
    pusha
    cld
    call interrupt_handler_13 
    popa
    iret

isr14:                     
    pusha
    cld
    call interrupt_handler_14 
    popa
    iret

isr15:                    
    pusha
    cld
    call interrupt_handler_15 
    popa
    iret

isr16:                     
    pusha
    cld
    call interrupt_handler_16 
    popa
    iret

isr17:                    
    pusha
    cld
    call interrupt_handler_17 
    popa
    iret

isr18:                     
    pusha
    cld
    call interrupt_handler_18
    popa
    iret

isr19:                    
    pusha
    cld
    call interrupt_handler_19
    popa
    iret

isr20:                     
    pusha
    cld
    call interrupt_handler_20 
    popa
    iret

isr21:                    
    pusha
    cld
    call interrupt_handler_21 
    popa
    iret

isr22:                     
    pusha
    cld
    call interrupt_handler_22
    popa
    iret

isr23:                    
    pusha
    cld
    call interrupt_handler_23 
    popa
    iret

isr24:                     
    pusha
    cld
    call interrupt_handler_24 
    popa
    iret

isr25:                    
    pusha
    cld
    call interrupt_handler_25 
    popa
    iret

isr26:                     
    pusha
    cld
    call interrupt_handler_26 
    popa
    iret

isr27:                    
    pusha
    cld
    call interrupt_handler_27 
    popa
    iret

isr28:                     
    pusha
    cld
    call interrupt_handler_28
    popa
    iret

isr29:                    
    pusha
    cld
    call interrupt_handler_29
    popa
    iret

isr30:                     
    pusha
    cld
    call interrupt_handler_30 
    popa
    iret

isr31:                    
    pusha
    cld
    call interrupt_handler_31 
    popa
    iret

load_idt:
    mov eax, [esp + 4]
    lidt [eax]
    ; sti
    ret

test_idt:
    INT 0x2
    ret;

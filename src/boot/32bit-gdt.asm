; GDT 

gdt_start:

gdt_null:

    ;   https://nasm.us/doc/nasmdoc3.html
    dd 0x0      ;   'dd' is 4 bytes
    dd 0x0 


; since we are little endian
; we put things backwards?

; https://wiki.osdev.org/Global_Descriptor_Table
gdt_code:       ;   the code segment descriptor 
    dw 0xffff   ;   limit is 0xfffff
    dw 0x0      ;   base 0 - 15

    db 0x0          ;   base 16 - 23
    db 10011010b    ;   Present (1 bit): 1
                    ;   Descriptor Privilege Level (2 bit): 0, ring 0 is highest privilege
                    ;   Descriptor type (1 bit): 1 for code and data segment
                    ;   Excutable bit (1 bits): if 1, code segment, if 0, data segment 
                    ;   Direction bit/Conforming bit (1 bit): 0. for details see link above
                    ;   Readable bit/writable bit(1 bit):  if this is a code segment, 1 makes the code segment readable       
                    ;                                      if this is a data segment, then 1 means writable for data segment, 
                    ;                                      readable is always allowed for data segment
                    ;   Accessed bit (1 bit): the CPU sets this to 1 when the segment is accessed 
                    ;                         so we can just set it to 0

    db 11001111b    ; 2nd flags , Limit ( bits 16 -19)
                    ; Graunilaiirty(bit): 1
                    ; Size: 1 means 32 bit protected mode
    db 0x0          ; Base ( bits 24 -31)
    
gdt_data:
    dw 0xffff           ;   limit is 0xfffff
    dw 0x0              ;   base 0 - 15
    db 0x0              ;   base 16 - 23
    db 10010010b        ;   Present (1 bit): 1                                                          1
                        ;   Descriptor Privilege Level (2 bit): 0, ring 0 is highest privilege          00
                        ;   Descriptor type (1 bit): 1 for code and data segment                        1
                        ;   Excutable bit (1 bits): if 1, code segment, if 0, data segment              0
                        ;   Direction bit/Conforming bit (1 bit): 0. for details see link above         0
                        ;   Readable bit/writable bit(1 bit):  if this is a code segment, 1 makes the code segment readable        1
                        ;                                      if this is a data segment, then 1 means writable for data segment, 
                        ;                                      readable is always allowed for data segment
                        ;   Accessed bit (1 bit): the CPU sets this to 1 when the segment is accessed
                        ;                         so we can just set it to 0                            0
    db 11001111b        ; 2nd flags , Limit ( bits 16 -19)
                        ; Graunilaiirty(bit): 1
                        ; Size: 1 means 32 bit protected mode
    db 0x0              ; Base ( bits 24 -31)
gdt_end:


gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start 


;   Define some handy constants for the GDT segment descriptor offsets, 
;   the segment registers will contain these values in protected mode. 
;   For example, when we set DS = 0x10 in protected mode, the CPU knows 
;   that we mean it to use the segment described at offset 0x10 (16 bytes);
;   in our GDT, which in our case is the DATAsegment 
;           0x0     -> NULL 
;           0x08    -> CODE 
;           0x10    -> DATA 

CODE_SEG equ gdt_code - gdt_start 
DATA_SEG equ gdt_data - gdt_start 
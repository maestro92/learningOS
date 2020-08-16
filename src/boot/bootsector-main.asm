;[org 0x7c00]    ; bootloader offset, dont need this since we are compiling to elf

KERNEL_OFFSET equ 0x1000    ; memory offset to which we will load our kernel

[bits 16]
boot_sector_start:

    mov [BOOT_DRIVE], dl    ;   when we boot, BIOS stores the boot dirve in the dl register
                            ;   we want to extract that number and put it into [BOOT_DRIVE]

    mov bp, 0x9000          ; set the stack safely away from us
    mov sp, bp

    mov bx, MSG_REAL_MODE
    call print
    call print_nl

    xor ax, ax              ; clear ax
    int 0x12                ; get the amount of KB from the BIOs
                            ; now AX contains the amount of KB in system recorded by BIOs

    call load_kernel    

    call switch_to_protected_mode
    jmp $   ;   this actually never gets executed 


%include "boot/real_mode_print_string.asm"
%include "boot/disk_load.asm"
%include "boot/32bit-gdt.asm"
%include "boot/32bit-print.asm"
%include "boot/32bit-switch.asm"

[bits 16]
load_kernel:
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

    call KERNEL_OFFSET      ; Now jump to the address of our loaded kernel code
    jmp $




BOOT_DRIVE   db 0
MSG_REAL_MODE  db "Started in 16-bit real mode", 0
MSG_PROTECTED_MODE  db "Loaded 32-bit protected mode", 0
MSG_LOAD_KERNEL  db "Loading kernel into memory.", 0

; Magic number
times 510 - ($-$$) db 0
dw 0xaa55

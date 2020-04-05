; [org 0x7c00]    ; bootloader offset, dont need this since we are compiling to elf

; KERNEL_OFFSET equ 0x1000    ; memory offset to which we will load our kernel

[bits 16]
boot_sector_start:

    mov bx, HELLO
    call print

    call print_nl

    mov bx, GOODBYE
    call print

    call print_nl

    mov dx, 0x12fe
    call print_hex
    jmp $   ;   this actually never gets executed 


%include "real_mode_print_string.asm"

BOOT_DRIVE   db 0

HELLO:
    db 'Hello, World', 0

GOODBYE:
    db 'Goodbye', 0


; Magic number
times 510 - ($-$$) db 0
dw 0xaa55

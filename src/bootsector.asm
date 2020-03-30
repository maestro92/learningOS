[org 0x7c00]

    mov [BOOT_DRIVE], dl    ;   when we boot, BIOS stores the boot dirve in the dl register
                            ;   we want to extract that number and put it into [BOOT_DRIVE]

    mov bx, HELLO 
    call print 
    call print_nl 

    mov bx, GOODBYE 
    call print 
    call print_nl

    mov dx, 0x12fe
    call print_hex
    call print_nl


    mov bp, 0x8000          ; set the stack safely away from us
    mov sp, bp

    mov bx, 0x9000          ; es:bx = 0x0000:0x9000 = 0x09000
    mov dh, 2               ; read 2 sectors
    mov dl, [BOOT_DRIVE]    ;   drive number (0=A:, 1=2nd floppy, 80h=drive 0, 81h=drive 1)
    call disk_load


    ; printing the outcomes
    mov dx, [0x9000]        ; retrieve the first loaded word, 0xdada
    call print_hex
    call print_nl

    mov dx, [0x9000 + 512]  ; first word from second loaded sector, 0xface
    call print_hex

    jmp $

%include "print_string.asm"
%include "disk_load.asm"

; data 
HELLO:
    db 'Hello World', 0

GOODBYE:
    db 'Goodbye', 0


;   Global variable
BOOT_DRIVE: db 0


; Magic number
times 510 - ($-$$) db 0
dw 0xaa55

; boot sector = sector 1 of cyl 0 of head 0 of hdd 0
; from now on = sector 2 ...
times 256 dw 0xdada ; sector 2 = 512 bytes
times 256 dw 0xface ; sector 3 = 512 bytes
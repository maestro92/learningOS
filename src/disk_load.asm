disk_load:
	pusha
	push dx 

	mov ah, 0x02		;	bios read sector function
	mov al, dh 			;	read dh sector 
	mov ch, 0x00		;	select cylinder 0 
	mov dh, 0x00		;	select head 0 
	mov cl, 0x02 		;	start reading from second sector 
	int 0x13 			;	BIOS interrupt 

	jc disk_error 		; 	jump if error 

	pop dx
	cmp dh, al 
	jne disk_error 
	popa
	ret 

disk_error:
	mov bx, DISK_ERROR_MSG 
	call print
	jmp $

DISK_ERROR_MSG	db "Disk read error!", 0
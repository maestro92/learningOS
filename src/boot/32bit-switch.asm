[bits 16]
switch_to_protected_mode:
	cli							; disable itnerrupts
	lgdt [gdt_descriptor]		; 2 load the GDT descriptor
	mov eax, cr0
	or eax, 0x1					; set PE (Protection Enable) bit in CR0 (Control Register 0)				
	mov cr0, eax

	jmp CODE_SEG:init_protected_mode		; do our far jump


[bits 32]

;	https://wiki.osdev.org/GDT_Tutorial

init_protected_mode:
   mov   ax, DATA_SEG 		; 0x10 points at the new data selector
   mov   ds, ax
   mov   ss, ax
   mov   es, ax
   mov   fs, ax
   mov   gs, ax

   mov ebp, 0x90000 	      ; update the stack again
   mov esp, ebp 		      ;	
   
   call begin_protected_mode	
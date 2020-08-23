The the setup is menteiond in OSDev3

Bochs will boot from floppy disk image in drive A. 

http://www.brokenthorn.com/Resources/OSDev3.html

In my case, we use qemu to boot from hard drive 










1.	we start off at Boot1.asm
as you can see the main steps are below:

essentially we are loading the bootloader2 to memory address 0x0500

and then we jump to the bootloader2 by calling 


				push    WORD 0x0050		<-----------
				push    WORD 0x0000
				retf

so lets see the code below:

				Stage1/Boot1.asm


				bits	16						; we are in 16 bit real mode

				org		0					; we will set regisers later

				start:	jmp	main					; jump to start of bootloader


				...
				...

				main:



				    ; read root directory into memory (7C00:0200)
				     
						mov     bx, 0x0200                            ; copy root dir above bootcode
						call    ReadSectors

					; Load root directory table	

					; Find stage 2

					; Load FAT

					; Load Stage 2



2.	Regarding loading file off disk, 

http://www.brokenthorn.com/Resources/OSDev5.html
We currently have the bootloader setup for a FAT12 File System. 
Windows will attempt to read certain tables (File Allocation Tables) from Sector 2 and 3.
we have to find the Starting Sector, Number of sectors, base address, etc. of a file in order to load it properly. 
This is the bases of loading files off disk.


Navigating the FAT12 File System


https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system

A File System is nothing more then a specification. It helps create the concept of a "file" on a disk.

As you know, a sector is 512 bytes in size. A file is stored across these sectors on disk. 
If the file exceeds 512 bytes, we have to give it more sectors. 
Because not all files are evenly 512 bytes in size, we will need to fill in the rest of the bytes 
(That the file doesnt use). Kind of like what we did for our bootloader.


FAT12 Filesystem - Theory

-	FAT12 has no support for hierarchical directories. This means there is only one directory-The Root Directory


what I realized is that, if you dont use a file system, you dont really know how many sectors to load.
unless you can create a disk that perfectly the number of sectors to hold your kernel.
but usually you first create an hard drive image, then partition it, and then create the number sections,
then finally put your kernel in. So its in reverse.




The Root Directory Table
Now, THIS will be important to us :)
The root directory is a table of 32 byte values that represent information reguarding file and directories. This 32 byte value uses the format:

Bytes 0-7 : DOS File name (Padded with spaces)
Bytes 8-10 : DOS File extension (Padded with spaces)
Bytes 11 : File attributes. This is a bit pattern:
Bit 0 : Read Only
Bit 1 : Hidden
Bit 2 : System
Bit 3 : Volume Label
Bit 4 : This is a subdirectory
Bit 5 : Archive
Bit 6 : Device (Internal use)
Bit 6 : Unused
Bytes 12 : Unused
Bytes 13 : Create time in ms
Bytes 14-15 : Created time, using the following format:
Bit 0-4 : Seconds (0-29)
Bit 5-10 : Minutes (0-59)
Bit 11-15 : Hours (0-23)
Bytes 16-17 : Created year in the following format:
Bit 0-4 : Year (0=1980; 127=2107
Bit 5-8 : Month (1=January; 12=December)
Bit 9-15 : Hours (0-23)
Bytes 18-19 : Last access date (Uses same format as above)
Bytes 20-21 : EA Index (Used in OS/2 and NT, dont worry about it)
Bytes 22-23 : Last Modified time (See byte 14-15 for format)
Bytes 24-25 : Last modified date (See bytes 16-17 for format)
Bytes 26-27 : First Cluster
Bytes 28-32 : File Size








				main:



				    ; read root directory into memory (7C00:0200)
				     
						mov     bx, 0x0200                            ; copy root dir above bootcode
						call    ReadSectors

					; Load root directory table	

					; Find stage 2

					; Load FAT

					; Load Stage 2








				     DONE:
				     
				          mov     si, msgCRLF
				          call    Print
					  	  mov	  dl, [bootdevice]
				          push    WORD 0x0050		<-----------
				          push    WORD 0x0000
				          retf

			          TIMES 510-($-$$) DB 0
			          DW 0xAA55


2.	the thing with retf is that 

        Transfers control from a procedure back to the instruction address
        saved on the stack.  n bytes is an optional number of bytes to
        release.  Far returns pop the IP followed by the CS, while near
        returns pop only the IP register.


so the 0050:0000 will take us to 0x0500


so the code above takes us to the file Stage2.asm





3.	so in Stage2.asm, you can see that our code is actually at 0x0500 is

				Stage2.asm


				bits	16

	------->	org 0x500

				jmp	main				; go to start



###############################################################################
########################### Stage2 Loading the Kernel #########################
###############################################################################

4.	lets look at how stage2.asm works 
essentially, our stage2.asm will be loading our kernel to to memory location 1MB mark


first you can see that call _EnableA20

so in stage2, we are calling BiosGetMemorySize64MB and BiosGetMemoryMap.

				main:

					;-------------------------------;
					;   Setup segments and stack	;
					;-------------------------------;

					cli	                   ; clear interrupts
					xor		ax, ax             ; null segments
					mov		ds, ax
					mov		es, ax
					mov		ax, 0x0000         ; stack begins at 0x9000-0xffff
					mov		ss, ax
					mov		sp, 0xFFFF
					sti	                   ; enable interrupts

					call	_EnableA20



So by EnablingA20, our program can now address more memory 
this is mentioned at the "Conclusion" of OSDev9


 				"We will add the FAT12 loading code to load our kernel. 
 				Now that A20 is enabled, we can load it at 1MB!"









5.	We will first look at how the kernel is loaded.
So OSDev11 Section "Finishing Stage2" talks about Loading and executing the Kernel

While we do want to load it to 1MB, we cannot do this directly yet, The reason is that becuz 
we are still in rea lmode. Because of this, we first need to load the image 
to a lower address first. After we switch into protected mode, we can copy our kernle to a new location.

This new location can be 1MB, or even 3GB if paging is enabled. 

We first load it to IMAGE_RMODE_BASE.


				main:

					;-------------------------------;
					;   Setup segments and stack	;
					;-------------------------------;



					call	LoadRoot		; Load root directory table
				 
					mov	ebx, 0			; BX:BP points to buffer to load to
	----------->   	mov	ebp, IMAGE_RMODE_BASE
					mov	Esi, ImageName		; our file to load
					call	LoadFile		; load our file
	----------->	MOV	dword [ImageSize], ecx	; size of kernel
					cmp	ax, 0			; Test for success
					je	EnterStage3		; yep--onto Stage 3!
					mov	si, msgFailure		; Nope--print error
					call	Puts16
					mov	ah, 0
					int     0x16                    ; await keypress
					int     0x19                    ; warm boot computer
					cli				; If we get here, something really went wong
					hlt

we also get the ImageSize 




if you look at common.inc, we define two constants 


				%ifndef _COMMON_INC_INCLUDED
				%define _COMMON_INC_INCLUDED

				; where the kernel is to be loaded to in protected mode
				%define IMAGE_PMODE_BASE 0x100000

				; where the kernel is to be loaded to in real mode
				%define IMAGE_RMODE_BASE 0x3000

				; kernel name
				ImageName     db "KRNL32  EXE"

				ImageSize     db 0

				%endif


So now our Kernle is loaded to IMAGE_RMODE_BASE:0. ImageSize contains the number of sectors loaded 
(The size of the kernel)

				
6.	So now we want ot execute inside of protected mode, all we need to do is jump or call it.
Since we want our kernel to be at 1 MB, we first need to copy it before we execute it. 


				bits 32
				 
				Stage3:
				 
					mov	ax, DATA_DESC		; set data segments to data selector (0x10)
					mov	ds, ax
					mov	ss, ax
					mov	es, ax
					mov	esp, 90000h		; stack begins from 90000h
				 
				; Copy kernel to 1MB (0x10000)
				 
				CopyImage:
				  	 mov	eax, dword [ImageSize]
				  	 movzx	ebx, word [bpbBytesPerSector]
				  	 mul	ebx
				  	 mov	ebx, 4
				  	 div	ebx
				   	 cld
				   	 mov    esi, IMAGE_RMODE_BASE
				   	 mov	edi, IMAGE_PMODE_BASE
				   	 mov	ecx, eax
				   	 rep	movsd                   ; copy image to its protected mode address
				 
					  call	CODE_DESC:IMAGE_PMODE_BASE; execute our kernel!













5.	so in EnterStage3, we will switch into protected mode				

	this is just standard stuff by setting the bit in CRO-0 to go into pmode

					;-------------------------------;
					;   Go into pmode               ;
					;-------------------------------;

				EnterStage3:

					cli	                           ; clear interrupts
					mov	eax, cr0                   ; set bit 0 in cr0--enter pmode
					or	eax, 1
					mov	cr0, eax

					jmp	CODE_DESC:Stage3                ; far jump to fix CS. Remember that the code selector is 0x8!




###############################################################################
########################### Stage3 Loading the Kernel #########################
###############################################################################

6.	then we see what happens in Stage3: As you can see, here we are in protected mode, 
so we want to run our code in 32 bits. 

				;******************************************************
				;	ENTRY POINT FOR STAGE 3
				;******************************************************

				bits 32

				BadImage db "*** FATAL: Invalid or corrupt kernel image. Halting system.", 0

				Stage3:

					;-------------------------------;
					;   Set registers				;
					;-------------------------------;

					mov	ax, DATA_DESC		; set data segments to data selector (0x10)
					mov	ds, ax
					mov	ss, ax
					mov	es, ax
					mov	esp, 90000h		; stack begins from 90000h


					...
					...

					jmp	EXECUTE




7.	then we call EXECUTE to execute the kernel




				EXECUTE:

					;---------------------------------------;
					;   Execute Kernel
					;---------------------------------------;

				    ; parse the programs header info structures to get its entry point

					add		ebx, 24
					mov		eax, [ebx]			; _IMAGE_FILE_HEADER is 20 bytes + size of sig (4 bytes)
					add		ebx, 20-4			; address of entry point
					mov		ebp, dword [ebx]		; get entry point offset in code section	
					add		ebx, 12				; image base is offset 8 bytes from entry point
					mov		eax, dword [ebx]		; add image base
					add		ebp, eax
					cli

					mov		eax, 0x2badb002			; multiboot specs say eax should be this
					mov		ebx, 0
					mov		edx, [ImageSize]
					


					call	ebp               	      ; Execute Kernel
					add		esp, 4



Lets look at how BiosGetMemorySize64MB and BiosGetMemoryMap works


8.	That calls into main.cpp which is in our kernel folder 

				SysCore/Kernel/main.cpp

				int _cdecl kmain (multiboot_info* bootinfo) {



				}

[bits 32]
[extern start] ; Define calling point. Must have same name as kernel.c 'main' function
call start ; Calls the C function. The linker will know where it is placed in memory. If I remember correctly c style compiler exports function adds an underscore before the function
jmp $

global load_paging_directory
global enable_paging
global flush_tlb


load_paging_directory:
    mov eax, [esp + 4]
    mov cr3, eax
    ret

enable_paging:
	mov eax, cr0
	or eax, 0x80000000
	mov cr0, eax
	ret

flush_tlb:
	mov eax, cr0
	or eax, 0x80000000
	mov cr0, eax
	ret


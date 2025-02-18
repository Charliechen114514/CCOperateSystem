%include "asm/boot.inc"             ; boot settings
%include "asm/gpt.inc"              ; gpt settings
%include "asm/detect_memory.inc"    ; memory detector settings
%include "asm/print.inc"            ; including for the print in preserved
%include "asm/pagetable.inc"        ; pagetable settings


; -------------------------------------------
;   Author Mentioned:
;       You can place a jmp here, but the base will
;       be offseted to 0x903, so remember this later
; -------------------------------------------
section loader vstart=LOADER_BASE_ADDR   ; Define loader section starting at LOADER_BASE_ADDR

; gpt table here, see the gpt.inc for better 
GPT_TABLE
; kernel info here
KERNEL_INFO

LOADER_STACK_TOP    equ LOADER_BASE_ADDR   ; Set stack top address for the loader

loader_start:

    xor ebx, ebx		      ; ebx should be 0
    mov edx, E820Magic	      
    mov di, ards_buf	      ;  see gpt sections for details
 .fetch_each_memory_info:	      ;
    mov eax, E820SubFunc	   ;
    mov ecx, 20		         ;  20 bytes each
    int 0x15
    ;  if 1, failed, so we check e801
    jc .try_e801   
    add di, cx              ; Increase di by 20 bytes to point to the new ARDS structure position in the buffer
    inc word [ards_nr]       ; Increment the ARDS count
    cmp ebx, 0               ; If ebx is 0 and cf is not 1, it means all ARDS have been returned, and we are at the last one
    jnz .fetch_each_memory_info
 
 ; Find the maximum value of (base_add_low + length_low) among all ARDS structures, which represents the largest memory capacity.
    mov cx, [ards_nr]        ; Loop through each ARDS structure, the loop runs for the number of ARDS structures
    mov ebx, ards_buf 
    xor edx, edx             ; Clear edx, which will hold the largest memory capacity
 .find_max_mem_area:        ; No need to check the type, the largest memory block is always usable
    mov eax, [ebx]           ; base_add_low
    add eax, [ebx+8]         ; length_low
    add ebx, 20              ; Move to the next ARDS structure in the buffer
    cmp edx, eax             ; Compare and find the maximum value, edx will always hold the largest memory capacity
    jge .next_ards
    mov edx, eax             ; edx will hold the total memory size
 
 .next_ards:
    loop .find_max_mem_area
    jmp .mem_get_ok
 
 ;------  int 15h ax = E801h Max support 4G  ------
 .try_e801:
    mov ax,E801Magic
    int 0x15
    jc .try88Method   ; same, if we failed, our last way :(
 
    mov cx,0x400	     
    mul cx 
    shl edx,16
    and eax,0x0000FFFF
    or edx,eax
    add edx, 0x100000    ; add 1MB
    mov esi,edx	         ;
 
 ;  transfer to byte storage
    xor eax,eax
    mov ax,bx		
    mov ecx, 0x10000	; => 64KB
    mul ecx		      
    add esi,eax		  
    mov edx,esi		   
    jmp .mem_get_ok
 
 ;-----------------  int 15h ah = 0x88 64M  ----------
 .try88Method: 
    ;int 15后，KB as the counts
    mov  ah, 0x88
    int  0x15
    jc .error_hlt
    and eax,0x0000FFFF
       
    mov cx, 0x400    
    mul cx
    shl edx, 16	     
    or edx, eax	    
    add edx,0x100000  
 
 .mem_get_ok:
    mov [total_mem_bytes], edx	 ; put it into the case, we will fetch it later:)

   ; open the A20
   in al, 0x92                          ; Read from port 0x92
   or al, 0000_0010b                    ; Set the A20 bit to enable it
   out 0x92, al                         ; Write the modified value back to port 0x92

   lgdt [GDT_PTR]                       ; Load the GDT pointer
   mov eax, cr0                         ; Read the control register CR0
   or eax, 0x0000_0001                  ; Set the PE bit to enable protected mode
   mov cr0, eax                         ; Write the updated value back to CR0
   jmp dword SELECTOR_CODE:protect_mode_start ; Jump to protected mode start

; memory check failed 
.error_hlt:		  
   hlt

; now this is 32bits mode 
[bits 32]                                ; Set 32-bit mode for the following code
protect_mode_start:
   mov ax, SELECTOR_DATA                ; Load the data selector into ax
   mov ds, ax                           ; Set ds to the value in ax

   mov es, ax                           ; Set es to the value in ax
   mov ss, ax                           ; Set ss to the value in ax

   mov esp, LOADER_STACK_TOP            ; Set the stack pointer to the loader stack top
   mov ax, SELECTOR_VIDEO               ; Load the video selector into ax
   mov gs, ax                           ; Set gs to the value in ax

   ; Create page directory and page tables, and initialize page memory bitmap
   call setup_page

   ; Store the descriptor table address and offset into memory GDT_PTR, then reload with new address
   sgdt [GDT_PTR]                      ; Store to the original gdt location

   ; Add 0xc0000000 to the segment base address in the video segment descriptor in the GDT
   mov ebx, [GDT_PTR + 2]  
   ; Video segment is the 3rd descriptor, each descriptor is 8 bytes, hence 0x18.
   ; The highest byte of the high 4 bytes of the segment descriptor is bits 31~24 of the segment base address
   or dword [ebx + 0x18 + 4], 0xc0000000     

   ; Add 0xc0000000 to the GDT base address to make it a high address in the kernel
   add dword [GDT_PTR + 2], 0xc0000000
   add esp, 0xc0000000                 ; Map the stack pointer to the kernel address

   ; Load the page directory address into cr3
   mov eax, PAGE_TABLE_POSITION
   mov cr3, eax

   ; Enable paging by setting the PG bit (bit 31) in cr0
   mov eax, cr0
   or eax, 0x80000000
   mov cr0, eax

   ; After enabling paging, reload the GDT with the new address
   lgdt [GDT_PTR]                      ; Reload

   mov esi, success_in_pro_mode
   mov cx, 4
   call protective_print_string

   Terminate

;-------------   Create Page Directory and Page Tables   ---------------
setup_page:
   ; First, clear the space occupied by the page directory byte by byte
      mov ecx, 4096
      mov esi, 0
   .clear_page_dir:
      mov byte [PAGE_TABLE_POSITION + esi], 0
      inc esi
      loop .clear_page_dir
      
   ; Start creating Page Directory Entries (PDE)
   .create_pde:                     ; Create Page Directory Entry
      mov eax, PAGE_TABLE_POSITION
      add eax, 0x1000               ; Now eax is the location and attributes of the first page table
      mov ebx, eax                  ; Assign ebx here to prepare for .create_pte, ebx is the base address.
   
   ;   Now store the address of the first page table in both directory entries 0 and 0xc00,
   ;   One page table can represent 4MB of memory, so addresses below 0xc03fffff and 0x003fffff point to the same page table,
   ;   This is to prepare for mapping addresses to kernel addresses
      or eax, PAGE_USER | PAGE_WRITE | PAGE_TABLE_P   ; Page directory entry attributes RW and P bits are 1, US is 1, indicating user attributes, accessible by all privilege levels.
      mov [PAGE_TABLE_POSITION + 0x0], eax            ; The first directory entry, write the location (0x101000) and attributes (3) of the first page table into the first directory entry in the page directory table
      mov [PAGE_TABLE_POSITION + 0xc00], eax          ; Each page table entry occupies 4 bytes, 0xc00 represents the directory entry occupied by the 768th page table, directory entries above 0xc00 are used for kernel space,
                                                         ; i.e., page tables 0xc0000000~0xffffffff totaling 1G belong to the kernel, 0x0~0xbfffffff totaling 3G belong to user processes.
      sub eax, 0x1000
      mov [PAGE_TABLE_POSITION + 4092], eax      ; Make the last directory entry point to the address of the page directory table itself
      
   ; Now create Page Table Entries (PTE)
      mov ecx, 256                     ; 1M low-end memory / 4k per page = 256
      mov esi, 0
      mov edx, PAGE_USER | PAGE_WRITE | PAGE_TABLE_P        ; Attributes are 7, US=1, RW=1, P=1
   .create_pte:                     ; Create Page Table Entry
      mov [ebx+esi*4],edx           ; Now ebx has been assigned the value 0x101000 through eax, which is the address of the first page table 
      add edx,4096
      inc esi
      loop .create_pte
      
   ; Create PDEs for other kernel page tables
      mov eax, PAGE_TABLE_POSITION
      add eax, 0x2000               ; Now eax is the location of the second page table
      or eax, PAGE_USER | PAGE_WRITE | PAGE_TABLE_P  ; Page directory entry attributes RW and P bits are 1, US=0
      mov ebx, PAGE_TABLE_POSITION
      mov ecx, 254                  ; Range is the number of directory entries from 769 to 1022
      mov esi, 769
   .create_kernel_pde:
      mov [ebx+esi*4], eax
      inc esi
      add eax, 0x1000
      loop .create_kernel_pde
      ret

; ----------------------------
;   functions that enters the protective mode
;   esi is the ptr points to the string position
;   cl is the line numbers
; ----------------------------
protective_print_string:
   ; reserved using registers
   push edi
   push eax

   mov eax, NEW_LINE_LENGTH
   xor edi, edi

   mul cl
   add edi, eax

start_display:
   lodsb
   ; test if is NULL
   or al, al
   jz finish_job

   mov byte [gs:di], al
   inc edi
   
   mov byte [gs:di], PRO_PRINT_STYLE
   inc edi
   jmp start_display
   
finish_job:    
   pop edi
   pop eax
   ret

success_in_pro_mode    db "We are now in protect mode!", 0
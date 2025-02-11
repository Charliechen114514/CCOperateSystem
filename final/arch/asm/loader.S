%include "asm/boot.inc"             ; boot settings
%include "asm/gpt.inc"              ; gpt settings
%include "asm/pagetable.inc"        ; pagetable settings
%include "asm/detect_memory.inc"    ; memory detector settings
%include "asm/print.inc"            ; including for the print in preserved

section loader vstart=LOADER_BASE_ADDR

GPT_TABLE

; this it's the place we load the operating system
loader_start:
;-------  int 15h eax = 0000E820h ,edx = 534D4150h ('SMAP') to fetch the memory  -------

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


;-----------------   Ready for Pm   -------------------
;  Open A20
;  Load GDT
;  Pressup protection mode
%define  A20_Port       0x92 
%define  ENABLE_A20     0000_0010B
;-----------------  Open A20  --------------------------
   in al,         A20_Port
   or al,         ENABLE_A20
   out A20_Port,  al

   ;-----------------  Load GDT  ----------------
   lgdt [GDT_PTR]

   ;-----------------  Pressup protection mode  ----------------
   mov eax, cr0
   ; use or to set bit
   or eax, 0x00000001
   mov cr0, eax

   jmp dword SELECTOR_CODE:p_mode_start ; we jump to the followings	     
.error_hlt:		  
   hlt

[bits 32]
p_mode_start:
   mov ax, SELECTOR_DATA
   mov ds, ax
   mov es, ax
   mov ss, ax
   mov esp,LOADER_STACK_TOP
   mov ax, SELECTOR_VIDEO
   mov gs, ax

; -------------------------   Load Kernel  ----------------------
   mov eax, KERNEL_BIN_SECTOR          ; Sector number where kernel.bin is located
   mov ebx, KERNEL_BIN_BASE_ADDR       ; Address to write the data after reading from disk
   mov ecx, KERNEL_SECTOR_LOAD_READ    ; Number of sectors to read

   call rd_disk_m_32

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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;  No need to flush the pipeline at this point  ;;;;;;;;;;;;;;;;;;;;;;;;
; Since we are always in 32-bit mode, in principle, there is no need to force a flush. After testing, the following two lines are not necessary.
; But just in case, they are added to avoid potential future issues.
   jmp SELECTOR_CODE:enter_kernel      ; Force flush the pipeline, update GDT
   enter_kernel:    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
   call kernel_init
   mov esp, 0xc009f000
   jmp KERNEL_ENTRY_POINT              ; when in bochs. try vb 0008:0xc0001500, if stop, ok!


;-----------------   Copy segments from kernel.bin to the compiled address   -----------
   kernel_init:
   xor eax, eax
   xor ebx, ebx                        ; ebx records the program header table address
   xor ecx, ecx                        ; cx records the number of program headers in the program header table
   xor edx, edx                        ; dx records the size of each program header, i.e., e_phentsize

   mov dx, [KERNEL_BIN_BASE_ADDR + 42] ; The attribute at offset 42 bytes in the file is e_phentsize, indicating the size of each program header
   mov ebx, [KERNEL_BIN_BASE_ADDR + 28] ; The attribute at offset 28 bytes in the file is e_phoff, indicating the offset of the first program header in the file
                                     ; Actually, this value is 0x34, but we read it here for caution
   add ebx, KERNEL_BIN_BASE_ADDR
   mov cx, [KERNEL_BIN_BASE_ADDR + 44] ; The attribute at offset 44 bytes in the file is e_phnum, indicating the number of program headers
.each_segment:
   cmp byte [ebx + 0], PT_NULL          ; If p_type equals PT_NULL, this program header is .
   je .PTNULL

; Push parameters for the memcpy function, parameters are pushed from right to left. Function prototype is similar to memcpy(dst,src,size)
   push dword [ebx + 16]               ; The attribute at offset 16 bytes in the program header is p_filesz, push as the third parameter for memcpy: size
   mov eax, [ebx + 4]                  ; The attribute at offset 4 bytes in the program header is p_offset
   add eax, KERNEL_BIN_BASE_ADDR       ; Add the physical address where kernel.bin is loaded, eax is the physical address of this segment
   push eax                            ; Push as the second parameter for memcpy: source address
   push dword [ebx + 8]                ; Push as the first parameter for memcpy: destination address, the attribute at offset 8 bytes in the program header is p_vaddr, which is the destination address
   call mem_cpy                        ; Call mem_cpy to copy the segment
   add esp,12                          ; Clean up the three parameters pushed onto the stack
   .PTNULL:
   add ebx, edx                        ; edx is the size of each program header, i.e., e_phentsize, so ebx points to the next program header 
   loop .each_segment
   ret

;----------  Byte-by-byte copy mem_cpy(dst,src,size) ------------
; Input: Three parameters on the stack (dst,src,size)
; Output: None
;---------------------------------------------------------
   mem_cpy:		      
   cld
   push ebp
   mov ebp, esp
   push ecx                        ; rep uses ecx, but ecx is still needed for the outer loop, so back it up first
   mov edi, [ebp + 8]              ; dst
   mov esi, [ebp + 12]             ; src
   mov ecx, [ebp + 16]             ; size
   rep movsb                       ; Byte-by-byte copy

   ; Restore environment
   pop ecx		
   pop ebp
   ret


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


;-------------------------------------------------------------------------------
; Function: Read n sectors from the hard disk
rd_disk_m_32:	 
; eax=LBA sector number
; ebx=Memory address to write data to
; ecx=Number of sectors to read  
;-------------------------------------------------------------------------------

   mov esi,eax           ; Backup eax
   mov di,cx             ; Backup sector count to di
; Read/write hard disk:
; Step 1: Set the number of sectors to read
   mov dx,0x1f2
   mov al,cl
   out dx,al            ; Number of sectors to read

   mov eax,esi          ; Restore ax

   ; Step 2: Store the LBA address in 0x1f3 ~ 0x1f6

   ; LBA address bits 7~0 to port 0x1f3
   mov dx,0x1f3                       
   out dx,al                          

   ; LBA address bits 15~8 to port 0x1f4
   mov cl,8
   shr eax,cl
   mov dx,0x1f4
   out dx,al

   ; LBA address bits 23~16 to port 0x1f5
   shr eax,cl
   mov dx,0x1f5
   out dx,al

   shr eax,cl
   and al,0x0f          ; LBA bits 24~27
   or al,0xe0           ; Set bits 7~4 to 1110, indicating LBA mode
   mov dx,0x1f6
   out dx,al

   ; Step 3: Write the read command 0x20 to port 0x1f7
   mov dx,0x1f7
   mov al,0x20                        
   out dx,al

;;;;;;;  Now, the hard disk controller will read cx sectors starting 
;;;;;;;  from the specified LBA address (eax), check the hard disk status, 
;;;;;;;  and if not busy, read out the data from these cx sectors

   ; Step 4: Check hard disk status
.not_ready:              ; Test the BSY bit of port 0x1f7 (status register)
   ; The same port, when writing, represents the command word, when reading, represents the hard disk status
   nop
   in al,dx
   and al,0x88          ; Bit 4 is 1 indicating the hard disk controller is ready for data transfer, bit 7 is 1 indicating the hard disk is busy
   cmp al,0x08
   jnz .not_ready       ; If not ready, continue waiting.

   ; Step 5: Read data from port 0x1f0
   mov ax, di           ; The following reads data from the hard disk port using insw is faster, but to demonstrate more command usage,
                        ; we use this method here, and will use insw and outsw later

   mov dx, 256          ; di is the number of sectors to read, each sector is 512 bytes, each read is a word, so di*512/2 times, hence di*256
   mul dx
   mov cx, ax	   
   mov dx, 0x1f0
.go_on_read:
   in ax,dx		
   mov [ebx], ax
   add ebx, 2
                        ; Since in real mode the offset address is 16 bits, using bx will only access offsets 0~FFFFh.
                        ; The loader's stack pointer is 0x900, bx points to the data output buffer, and is 16 bits,
                        ; After exceeding 0xffff, the bx part will start from 0, so when the number of sectors to read is too large, and the address to write exceeds the range of bx,
                        ; The data read from the hard disk will overwrite 0x0000~0xffff,
                        ; Causing the stack to be corrupted, so when ret returns, the return address is corrupted, and is no longer the previous correct address,
                        ; So the program will error, and it is not known where it will go.
                        ; So replace bx with ebx to point to the buffer, so the generated machine code will have 0x66 and 0x67 in front to reverse.
                        ; 0X66 is used to reverse the default operand size! 0X67 is used to reverse the default addressing mode.
                        ; When the CPU is in 16-bit mode, it will naturally assume that the operand and addressing are 16-bit, when in 32-bit mode,
                        ; It will also assume that the instruction to be executed is 32-bit.
                        ; When we use the addressing mode or operand size of another mode in either mode (let's assume 16-bit mode uses 16-bit byte operands,
                        ; 32-bit mode uses 32-byte operands), the compiler will add 0x66 or 0x67 in front of the instruction for us,
                        ; Temporarily change the current CPU mode to the other mode.
                        ; Assuming running in 16-bit mode, when encountering 0X66, the operand size becomes 32-bit.
                        ; Assuming running in 32-bit mode, when encountering 0X66, the operand size becomes 16-bit.
                        ; Assuming running in 16-bit mode, when encountering 0X67, the addressing mode becomes 32-bit addressing
                        ; Assuming running in 32-bit mode, when encountering 0X67, the addressing mode becomes 16-bit addressing.

   loop .go_on_read
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